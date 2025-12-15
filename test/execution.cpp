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

#if THES_X86_64
  fmt::print("x86-64\n");
#endif
#if THES_ARM64
  fmt::print("ARM64\n");
#endif

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
#if THES_WINDOWS || THES_LINUX
  const auto logical_effi = std::ranges::to<std::vector<thes::CpuInfo>>(
    thes::CpuInfo::logical(thes::EfficiencyClass::efficiency));
  fmt::print("{}x logical efficiency: {}\n", logical_effi.size(), logical_effi);
  const auto logical_perf = std::ranges::to<std::vector<thes::CpuInfo>>(
    thes::CpuInfo::logical(thes::EfficiencyClass::performance));
  fmt::print("{}x logical performance: {}\n", logical_perf.size(), logical_perf);
#endif

  const auto physical = std::ranges::to<std::vector<thes::CpuInfo>>(thes::CpuInfo::physical());
  fmt::print("{}x physical: {}\n", physical.size(), physical);
#if THES_WINDOWS || THES_LINUX
  const auto physical_effi = std::ranges::to<std::vector<thes::CpuInfo>>(
    thes::CpuInfo::physical(thes::EfficiencyClass::efficiency));
  fmt::print("{}x physical efficiency: {}\n", physical_effi.size(), physical_effi);
  const auto physical_perf = std::ranges::to<std::vector<thes::CpuInfo>>(
    thes::CpuInfo::physical(thes::EfficiencyClass::performance));
  fmt::print("{}x physical performance: {}\n", physical_perf.size(), physical_perf);
#endif

  const auto physical_part =
    std::ranges::to<std::vector<thes::CpuInfo>>(thes::CpuInfo::physical_part(0, 2));
  fmt::print("{}x physical 0/2: {}\n", physical_part.size(), physical_part);
#if THES_LINUX || THES_WINDOWS
  const auto physical_part_effi = std::ranges::to<std::vector<thes::CpuInfo>>(
    thes::CpuInfo::physical_part(thes::EfficiencyClass::efficiency, 0, 2));
  fmt::print("{}x physical efficiency 0/2: {}\n", physical_part_effi.size(), physical_part_effi);
  const auto physical_part_perf = std::ranges::to<std::vector<thes::CpuInfo>>(
    thes::CpuInfo::physical_part(thes::EfficiencyClass::performance, 0, 2));
  fmt::print("{}x physical performance 0/2: {}\n", physical_part_perf.size(), physical_part_perf);
#endif

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
