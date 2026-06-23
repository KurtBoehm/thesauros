// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <cstdio>
#include <exception>
#include <functional>
#include <ranges>
#include <vector>

#include "thesauros/algorithms.hpp"
#include "thesauros/execution.hpp"
#include "thesauros/format.hpp"
#include "thesauros/macropolis/platform.hpp"
#include "thesauros/resources.hpp"
#include "thesauros/test.hpp"
#include "thesauros/types/type-name.hpp"

#if THES_LINUX || THES_WINDOWS
#include <chrono>
#include <thread>

#include "ankerl/unordered_dense.h"
#endif

int main() try {
  using Type = int;

  std::vector<Type> values{5, 8, 9, 4, 2, -10, 22};
  std::vector<Type> scanned(values.size());

  std::vector<Type> scanned_ref{};
  scanned_ref.reserve(values.size());
  for (Type accum = 0; const Type v : values) {
    accum += v;
    scanned_ref.push_back(accum);
  }

#if THES_X86_64
  fmt::print("x86-64\n\n");
#endif
#if THES_ARM64
  fmt::print("ARM64\n\n");
#endif

  auto run_scan = [&](auto pool) {
    thes::LinearExecutionPolicy expo{pool};

    thes::transform_inclusive_scan(
      expo, values.begin(), values.end(), scanned.begin(), std::plus<>{}, [](Type v) { return v; },
      Type{0});

    THES_ALWAYS_ASSERT(scanned == scanned_ref);
  };

  run_scan(thes::FixedStdThreadPool{2});
  run_scan(thes::FixedOpenMpThreadPool{2});

  const auto logical = std::ranges::to<std::vector<thes::CpuInfo>>(thes::CpuInfo::logical());
  fmt::print("{}× logical: {}\n", logical.size(), logical);
  // E cores
  const auto logical_effi = std::ranges::to<std::vector<thes::CpuInfo>>(
    thes::CpuInfo::logical(thes::EfficiencyClass::efficiency));
  fmt::print("{}× logical efficiency: {}\n", logical_effi.size(), logical_effi);
  // M cores
  const auto logical_medi = std::ranges::to<std::vector<thes::CpuInfo>>(
    thes::CpuInfo::logical(thes::EfficiencyClass::medium));
  fmt::print("{}× logical medium: {}\n", logical_medi.size(), logical_medi);
  // P cores
  const auto logical_perf = std::ranges::to<std::vector<thes::CpuInfo>>(
    thes::CpuInfo::logical(thes::EfficiencyClass::performance));
  fmt::print("{}× logical performance: {}\n", logical_perf.size(), logical_perf);
  fmt::print("\n");

  const auto physical = std::ranges::to<std::vector<thes::CpuInfo>>(thes::CpuInfo::physical());
  fmt::print("{}× physical: {}\n", physical.size(), physical);
  // E cores
  const auto physical_effi = std::ranges::to<std::vector<thes::CpuInfo>>(
    thes::CpuInfo::physical(thes::EfficiencyClass::efficiency));
  fmt::print("{}× physical efficiency: {}\n", physical_effi.size(), physical_effi);
  // M cores
  const auto physical_medi = std::ranges::to<std::vector<thes::CpuInfo>>(
    thes::CpuInfo::physical(thes::EfficiencyClass::medium));
  fmt::print("{}× physical medium: {}\n", physical_medi.size(), physical_medi);
  // P cores
  const auto physical_perf = std::ranges::to<std::vector<thes::CpuInfo>>(
    thes::CpuInfo::physical(thes::EfficiencyClass::performance));
  fmt::print("{}× physical performance: {}\n", physical_perf.size(), physical_perf);
  fmt::print("\n");

  const auto physical_part =
    std::ranges::to<std::vector<thes::CpuInfo>>(thes::CpuInfo::physical_part(0, 2));
  fmt::print("{}× physical 0/2: {}\n", physical_part.size(), physical_part);
  // E cores
  const auto physical_part_effi = std::ranges::to<std::vector<thes::CpuInfo>>(
    thes::CpuInfo::physical_part(thes::EfficiencyClass::efficiency, 0, 2));
  fmt::print("{}× physical efficiency 0/2: {}\n", physical_part_effi.size(), physical_part_effi);
  // M cores
  const auto physical_part_medi = std::ranges::to<std::vector<thes::CpuInfo>>(
    thes::CpuInfo::physical_part(thes::EfficiencyClass::medium, 0, 2));
  fmt::print("{}× physical medium 0/2: {}\n", physical_part_medi.size(), physical_part_medi);
  // P cores
  const auto physical_part_perf = std::ranges::to<std::vector<thes::CpuInfo>>(
    thes::CpuInfo::physical_part(thes::EfficiencyClass::performance, 0, 2));
  fmt::print("{}× physical performance 0/2: {}\n", physical_part_perf.size(), physical_part_perf);

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

    THES_ALWAYS_ASSERT(post_set == Ids(info_ids.begin(), info_ids.end()));

    thread.join();
  }
#endif
} catch (const std::exception& ex) {
  fmt::print(stderr, "Unhandled std::exception: type={}; what={}\n",
             thes::demangle(typeid(ex).name()), ex.what());
}
