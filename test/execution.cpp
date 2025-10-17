// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <functional>
#include <ranges>
#include <thread>
#include <vector>

#include "ankerl/unordered_dense.h"

#include "thesauros/algorithms.hpp"
#include "thesauros/execution.hpp"
#include "thesauros/format.hpp"
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

  const std::vector<thes::CpuInfo> logical(std::from_range, thes::CpuInfo::logical());
  fmt::print("{}x logical: {}\n", logical.size(), logical);

  const std::vector<thes::CpuInfo> physical(std::from_range, thes::CpuInfo::physical());
  fmt::print("{}x physical: {}\n", physical.size(), physical);

  const std::vector<thes::CpuInfo> physical_part(std::from_range,
                                                 thes::CpuInfo::physical_part(0, 2));
  fmt::print("{}x physical 0/2: {}\n", physical_part.size(), physical_part);

  {
    using Ids = ankerl::unordered_dense::set<thes::CpuSet::Id>;

    const auto infos = physical | std::views::take(2);
    const auto info_ids =
      infos | std::views::transform([](const thes::CpuInfo& info) { return info.id; });
    std::thread thread{[] { fmt::print("thread started\n"); }};

    const auto ante = thes::CpuSet::affinity(thread).cpu_ids();
    fmt::print("set before: {}\n", ante);
    const auto logical_ids =
      logical | std::views::transform([](const thes::CpuInfo& info) { return info.id; });
    THES_ASSERT((ante == Ids{} || ante == Ids{logical_ids.begin(), logical_ids.end()}));

    thes::set_affinity(thread, thes::CpuSet::from_cpu_infos(infos)).value();

    const auto post = thes::CpuSet::affinity(thread).cpu_ids();
    fmt::print("set after: {}\n", post);
    THES_ASSERT(post == Ids(info_ids.begin(), info_ids.end()));

    thread.join();
  }
}
