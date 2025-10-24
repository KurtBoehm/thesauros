// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <chrono>
#include <functional>
#include <ranges>
#include <thread>
#include <vector>

#include "ankerl/unordered_dense.h"

#include "thesauros/algorithms.hpp"
#include "thesauros/execution.hpp"
#include "thesauros/format.hpp"
#include "thesauros/macropolis/platform.hpp"
#include "thesauros/resources.hpp"
#include "thesauros/test.hpp"

int main() {
  using Type = int;

  std::vector<Type> values{5, 8, 9, 4, 2, -10, 22};
  std::vector<Type> scanned(values.size());
  std::vector<Type> scanned_ref{};
  for (Type accum = 0; const Type v : values) {
    accum += v;
    scanned_ref.push_back(accum);
  }

  auto op = [&](auto pool) {
    thes::LinearExecutionPolicy expo{pool};

    thes::transform_inclusive_scan(
      expo, values.begin(), values.end(), scanned.begin(), std::plus<>{}, [](Type v) { return v; },
      Type{0});

    THES_ASSERT(scanned == scanned_ref);
  };
  op(thes::FixedStdThreadPool{2});
  op(thes::FixedOpenMpThreadPool{2});

  const auto logical = std::ranges::to<std::vector<thes::CpuInfo>>(thes::CpuInfo::logical());
  fmt::print("{}x logical: {}\n", logical.size(), logical);

  const auto physical = std::ranges::to<std::vector<thes::CpuInfo>>(thes::CpuInfo::physical());
  fmt::print("{}x physical: {}\n", physical.size(), physical);

  const auto physical_part =
    std::ranges::to<std::vector<thes::CpuInfo>>(thes::CpuInfo::physical_part(0, 2));
  fmt::print("{}x physical 0/2: {}\n", physical_part.size(), physical_part);

#if THES_LINUX || THES_WINDOWS
  {
    using Ids = ankerl::unordered_dense::set<thes::CpuSet::Id>;

    const auto infos = physical | std::views::take(2);
    const auto info_ids =
      infos | std::views::transform([](const thes::CpuInfo& info) { return info.id; });
    std::thread thread{[] {
      std::this_thread::sleep_for(std::chrono::seconds{1});
      fmt::print("stop thread\n");
    }};

    auto ante = thes::CpuSet::affinity(thread).cpu_ids();
    fmt::print("set before: {}\n", ante);

    thes::set_affinity(thread, thes::CpuSet::from_cpu_infos(infos)).value();

    auto post = thes::CpuSet::affinity(thread).cpu_ids();
    const Ids post_set(post.begin(), post.end());
    fmt::print("set after: {}\n", post);
    THES_ASSERT(post_set == Ids(info_ids.begin(), info_ids.end()));

    thread.join();
  }
#endif
}
