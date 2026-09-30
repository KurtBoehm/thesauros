// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <iterator>
#include <random>
#include <string_view>

#include "thesauros/benchmark.hpp"
#include "thesauros/containers/array/fixed.hpp"
#include "thesauros/execution.hpp"
#include "thesauros/format.hpp"
#include "thesauros/math/integer-cast.hpp"
#include "thesauros/ranges/as-sized.hpp"
#include "thesauros/ranges/indices.hpp"
#include "thesauros/resources/cpu-info.hpp"
#include "thesauros/types/primitives.hpp"
#include "thesauros/types/type-tag.hpp"
#include "thesauros/utility/index-segmentation.hpp"

#include "executors/fixed-omp-thread-pool.hpp"
#include "executors/fixed-std-thread-pool.hpp"

int main() {
  using namespace thes::primitives;

  static constexpr std::size_t max_size = 1UZ << 28UZ;

  const auto cores = thes::ranges::as_sized(thes::CpuInfo::physical());
  const auto core_num = *thes::safe_cast<std::size_t>(std::ranges::distance(cores));

  thes::FixedArray<f64> a(max_size);
  thes::FixedArray<f64> b(max_size);
  thes::FixedArray<f64> c(max_size);
  std::mt19937_64 gen{};
  std::uniform_real_distribution<double> dist{};
  for (const std::size_t i : thes::views::indices(max_size)) {
    a[i] = dist(gen);
    b[i] = dist(gen);
    c[i] = dist(gen);
  }

  const auto work = [&]<typename Pool>(std::size_t size, thes::TypeTag<Pool> /*tag*/) {
    thes::UniformIndexSegmenter<std::size_t, std::size_t> seg{size, core_num};
    auto pool = Pool::from_cpu_infos(core_num, cores);
    return thes::bench::run([&] {
      pool.execute([&](std::size_t t) noexcept {
        const auto segment = seg.segment_range(t);
        for (const std::size_t i : segment) {
          c[i] = a[i] + b[i];
        }
      });
    });
  };

  fmt::print("pool       size iterations       s/it\n");

  for (std::size_t s = 0; s <= max_size; s = (s == 0) ? 1 : (2 * s)) {
    const auto res_std = work(s, thes::type_tag<thes::FixedStdThreadPool>);
    const auto res_omp = work(s, thes::type_tag<thes::FixedOpenMpThreadPool>);
    const auto res_opt = work(s, thes::type_tag<thes::FixedThreadPool>);

    const auto print = [&](std::string_view label, const thes::bench::Result& res) {
      fmt::print("{}  {:>10} {:>10} {:.4e}\n", label, s, res.iterations,
                 std::chrono::duration<double>{res.total_duration}.count() /
                   static_cast<double>(res.iterations));
    };

    print("std", res_std);
    print("omp", res_omp);
    print("opt", res_opt);
  }
}
