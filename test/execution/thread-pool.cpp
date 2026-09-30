// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <ranges>
#include <thread>
#include <vector>

#include "thesauros/execution.hpp"
#include "thesauros/format.hpp"
#include "thesauros/math/integer-cast.hpp"
#include "thesauros/ranges/as-sized.hpp"
#include "thesauros/ranges/indices.hpp"
#include "thesauros/resources.hpp"
#include "thesauros/test.hpp"
#include "thesauros/types/empty.hpp"

// The distance between the counters written by different threads, which keeps them on separate
// cache lines so that the benchmark measures the dispatch rather than false sharing.
constexpr std::size_t stride = 16;

constexpr std::size_t regions = 128;

int main() {
  const auto hardware =
    std::max(*thes::safe_cast<std::size_t>(std::thread::hardware_concurrency()), 1UZ);
  const std::size_t max_threads = std::min(hardware, 8UZ);

  //================================================================================================
  // Correctness
  //================================================================================================

  // Every thread index runs exactly once per region, both when the threads spin and when they park
  // immediately.
  for (const std::size_t spin : {0UZ, thes::FixedThreadPool::default_spin_count}) {
    for (const std::size_t size : thes::views::indices(1UZ, max_threads + 1)) {
      const thes::FixedThreadPool pool{size, thes::Empty{}, spin};
      THES_ALWAYS_ASSERT(pool.thread_num() == size);

      std::vector<std::size_t> counts(size * stride, 0);
      for ([[maybe_unused]] const std::size_t r : thes::views::indices(regions)) {
        pool.execute([&counts](std::size_t index) noexcept { counts[index * stride] += 1; });
      }
      for (const std::size_t index : thes::views::indices(size)) {
        THES_ALWAYS_ASSERT(counts[index * stride] == regions);
      }

      // Only the requested prefix of the thread indices participates.
      for (const std::size_t used : thes::views::indices(size + 1)) {
        std::vector<std::size_t> partial(size * stride, 0);
        pool.execute([&partial](std::size_t index) noexcept { partial[index * stride] += 1; },
                     used);
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
    pool.execute(
      [&ids](std::size_t index) noexcept { ids[index * stride] = std::this_thread::get_id(); });
    THES_ALWAYS_ASSERT(ids[0] == std::this_thread::get_id());
    for (const std::size_t index : thes::views::indices(1UZ, max_threads)) {
      THES_ALWAYS_ASSERT(ids[index * stride] != std::this_thread::get_id());
    }
  }

  //================================================================================================
  // Pinned threads, which come last because they restrict the calling thread as well
  //================================================================================================

  {
    const auto cpus = thes::ranges::as_sized(thes::CpuInfo::physical());
    const std::size_t size = std::min(cpus.size(), 2UZ);
    if (size > 0) {
      const auto pool = thes::FixedThreadPool::from_cpu_infos(size, cpus | std::views::take(size));
      std::vector<std::size_t> counts(size * stride, 0);
      pool.execute([&counts](std::size_t index) noexcept { counts[index * stride] += 1; });
      for (const std::size_t index : thes::views::indices(size)) {
        THES_ALWAYS_ASSERT(counts[index * stride] == 1);
      }
      fmt::print("pinned to {} of the {} physical CPUs\n", size, cpus.size());
    }
  }
}
