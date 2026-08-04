// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <ranges>
#include <ratio>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <vector>

#include "thesauros/execution.hpp"
#include "thesauros/format.hpp"
#include "thesauros/ranges/indices.hpp"
#include "thesauros/resources.hpp"
#include "thesauros/test.hpp"
#include "thesauros/types/empty.hpp"
#include "thesauros/types/type-name.hpp"

// The distance between the counters written by different threads, which keeps them on separate
// cache lines so that the benchmark measures the dispatch rather than false sharing.
constexpr std::size_t stride = 16;

constexpr std::size_t regions = 128;
constexpr std::size_t bench_warmup = 1024;
constexpr std::size_t bench_regions = 8192;

int main() try {
  const auto hardware = std::max(std::size_t{std::thread::hardware_concurrency()}, std::size_t{1});
  const std::size_t max_threads = std::min(hardware, std::size_t{8});

  //================================================================================================
  // Correctness
  //================================================================================================

  // Every thread index runs exactly once per region, both when the threads spin and when they park
  // immediately.
  for (const std::size_t spin : {std::size_t{0}, thes::FixedThreadPool::default_spin_count}) {
    for (const std::size_t size : thes::views::indices(std::size_t{1}, max_threads + 1)) {
      const thes::FixedThreadPool pool{size, thes::Empty{}, spin};
      THES_ALWAYS_ASSERT(pool.thread_num() == size);

      std::vector<std::size_t> counts(size * stride, 0);
      for ([[maybe_unused]] const std::size_t r : thes::views::indices(regions)) {
        pool.execute([&counts](std::size_t index) { counts[index * stride] += 1; });
      }
      for (const std::size_t index : thes::views::indices(size)) {
        THES_ALWAYS_ASSERT(counts[index * stride] == regions);
      }

      // Only the requested prefix of the thread indices participates.
      for (const std::size_t used : thes::views::indices(size + 1)) {
        std::vector<std::size_t> partial(size * stride, 0);
        pool.execute([&partial](std::size_t index) { partial[index * stride] += 1; }, used);
        for (const std::size_t index : thes::views::indices(size)) {
          THES_ALWAYS_ASSERT(partial[index * stride] == std::size_t{index < used});
        }
      }
    }
  }

  // Index 0 runs on the calling thread, which is what saves a pair of context switches per region.
  {
    const thes::FixedThreadPool pool{max_threads};
    std::vector<std::thread::id> ids(max_threads * stride);
    pool.execute([&ids](std::size_t index) { ids[index * stride] = std::this_thread::get_id(); });
    THES_ALWAYS_ASSERT(ids[0] == std::this_thread::get_id());
    for (const std::size_t index : thes::views::indices(std::size_t{1}, max_threads)) {
      THES_ALWAYS_ASSERT(ids[index * stride] != std::this_thread::get_id());
    }
  }

  // An exception thrown by any thread index escapes `execute` and leaves the pool usable.
  {
    const thes::FixedThreadPool pool{max_threads};
    for (const std::size_t thrower : thes::views::indices(max_threads)) {
      bool caught = false;
      try {
        pool.execute([thrower](std::size_t index) {
          if (index == thrower) {
            throw std::runtime_error{"task"};
          }
        });
      } catch (const std::runtime_error& ex) {
        caught = std::string_view{ex.what()} == "task";
      }
      THES_ALWAYS_ASSERT(caught);
    }

    std::vector<std::size_t> counts(max_threads * stride, 0);
    pool.execute([&counts](std::size_t index) { counts[index * stride] += 1; });
    for (const std::size_t index : thes::views::indices(max_threads)) {
      THES_ALWAYS_ASSERT(counts[index * stride] == 1);
    }
  }

  //================================================================================================
  // The latency of an empty parallel region
  //================================================================================================

  fmt::print("Mean wall time of a parallel region doing nothing but one store per thread,\n"
             "over {} regions after {} warm-up regions:\n\n",
             bench_regions, bench_warmup);

  std::vector<std::size_t> sizes{std::size_t{2}, max_threads};
  std::ranges::sort(sizes);
  const auto duplicates = std::ranges::unique(sizes);
  sizes.erase(duplicates.begin(), duplicates.end());
  std::erase_if(sizes, [max_threads](std::size_t size) { return size > max_threads; });

  for (const std::size_t size : sizes) {
    fmt::print("{} threads\n", size);

    auto bench = [size](std::string_view name, const auto& pool) {
      std::vector<std::size_t> sink(size * stride, 0);
      auto region = [&sink](std::size_t index) { sink[index * stride] += 1; };

      for ([[maybe_unused]] const std::size_t r : thes::views::indices(bench_warmup)) {
        pool.execute(region);
      }
      const auto start = std::chrono::steady_clock::now();
      for ([[maybe_unused]] const std::size_t r : thes::views::indices(bench_regions)) {
        pool.execute(region);
        // Without this, the regions of the sequential executor are merged into one loop.
        __asm__ __volatile__("" ::: "memory"); // NOLINT
      }
      const auto elapsed = std::chrono::steady_clock::now() - start;

      const auto nanos = std::chrono::duration<double, std::nano>(elapsed).count();
      fmt::print("  {:<22} {:9.1f} ns  ({} regions ran)\n", name, nanos / double(bench_regions),
                 sink[0]);
    };

    bench("FixedThreadPool", thes::FixedThreadPool{size});
    bench("FixedStdThreadPool", thes::FixedStdThreadPool{size});
    try {
      bench("FixedOpenMpThreadPool", thes::FixedOpenMpThreadPool{size});
    } catch (const std::exception& ex) {
      fmt::print("  {:<22} unavailable: {}\n", "FixedOpenMpThreadPool", ex.what());
    }
    bench("SequentialExecutor", thes::SequentialExecutor{});
    fmt::print("\n");
  }

  //================================================================================================
  // Pinned threads, which come last because they restrict the calling thread as well
  //================================================================================================

  {
    const auto cpus = std::ranges::to<std::vector<thes::CpuInfo>>(thes::CpuInfo::physical());
    const std::size_t size = std::min(cpus.size(), std::size_t{2});
    if (size > 0) {
      const auto pool = thes::FixedThreadPool::from_cpu_infos(size, cpus | std::views::take(size));
      std::vector<std::size_t> counts(size * stride, 0);
      pool.execute([&counts](std::size_t index) { counts[index * stride] += 1; });
      for (const std::size_t index : thes::views::indices(size)) {
        THES_ALWAYS_ASSERT(counts[index * stride] == 1);
      }
      fmt::print("pinned to {} of the {} physical CPUs\n", size, cpus.size());
    }
  }
} catch (const std::exception& ex) {
  fmt::print(stderr, "Unhandled std::exception: type={}; what={}\n",
             thes::demangle(typeid(ex).name()), ex.what());
  return 1;
}
