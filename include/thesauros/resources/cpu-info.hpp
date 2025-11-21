// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_RESOURCES_CPU_INFO_HPP
#define INCLUDE_THESAUROS_RESOURCES_CPU_INFO_HPP

#include <cassert>
#include <cstddef>
#include <ranges>

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/macropolis/platform.hpp"
#include "thesauros/ranges/iota.hpp"
#include "thesauros/utility/index-segmentation.hpp"

#if THES_LINUX
#include <algorithm>
#include <filesystem>
#include <string_view>
#include <utility>
#include <vector>

#include "thesauros/charconv/string-convert.hpp"
#include "thesauros/containers/array.hpp"
#include "thesauros/io/file-reader.hpp"
#elif THES_APPLE
#include <stdexcept>

#include <sys/sysctl.h>
#elif THES_WINDOWS
#include <memory>

#include <minwindef.h>
#include <processthreadsapi.h>
#include <winnt.h>

#include "ankerl/unordered_dense.h"

#include "thesauros/types/primitives.hpp"
#endif

namespace thes {
enum struct EfficiencyClass : u8 { any, efficiency, performance };

#if THES_LINUX
template<typename TChars>
auto cpu_range(TChars&& full) {
  const std::string_view fullsv{full.data(), full.size()};
  const auto size = full.size() - size_t(fullsv.ends_with("\n"));
  return std::forward<TChars>(full) | std::views::take(size) | std::views::split(',') |
         std::views::transform([](auto substr) {
           std::string_view subsv{substr.data(), substr.size()};
           const std::size_t hyphen = subsv.find('-');
           const auto first = string_to_integral<std::size_t>(subsv.substr(0, hyphen)).value();
           if (hyphen == std::string_view::npos) {
             return range(first, first + 1);
           }
           assert(subsv.find('-', hyphen + 1) == std::string_view::npos);
           const auto second = string_to_integral<std::size_t>(subsv.substr(hyphen + 1)).value();
           return range(first, second + 1);
         }) |
         std::views::join;
}

struct CpuInfo {
  static constexpr auto cpu_path_cstr = "/sys/devices/system/cpu/";

  std::size_t id;

  [[nodiscard]] auto sys_folder() const {
    return std::filesystem::path{cpu_path_cstr} / fmt::format("cpu{}", id) / "topology";
  }

  [[nodiscard]] auto core_cpus() const {
    return cpu_range(read_file<DynamicArray<char>>(sys_folder() / "core_cpus_list"));
  }

  static auto logical() {
    const auto present_path = std::filesystem::path{cpu_path_cstr} / "present";
    return cpu_range(read_file<DynamicArray<char>>(present_path)) |
           std::views::transform([](std::size_t i) { return CpuInfo{i}; });
  }

  static auto physical() {
    return logical() | std::views::filter([&](const CpuInfo& cpu) {
             return cpu.id == std::ranges::min(cpu.core_cpus());
           });
  }

  static auto physical_part(std::size_t idx, std::size_t num) {
    auto one_per_core = std::ranges::to<std::vector<CpuInfo>>(CpuInfo::physical());
    const auto subsizes = UniformIndexSegmenter{one_per_core.size(), num}.segment_range(idx);
    return std::move(one_per_core) | std::views::drop(subsizes.begin_value()) |
           std::views::take(subsizes.size());
  }
};
#elif THES_APPLE
template<typename T>
inline T read_sysctl(const char* name) {
  T value;
  std::size_t size = sizeof(T);
  const int ret = sysctlbyname(name, &value, &size, nullptr, 0);
  if (ret != 0) {
    throw std::runtime_error{fmt::format("sysctlbyname({}) failed: {}", name, ret)};
  }
  return value;
}
struct CpuInfo {
  std::size_t id;

  static auto logical() {
    const auto logical_cores = *safe_cast<std::size_t>(read_sysctl<int>("hw.logicalcpu"));
    return range(logical_cores) | std::views::transform([](std::size_t i) { return CpuInfo{i}; });
  }
  // Warning: In contrast to the Linux version, `logical` and `physical` do not share an index set!
  static auto physical() {
    const auto physical_cores = *safe_cast<std::size_t>(read_sysctl<int>("hw.physicalcpu"));
    return range(physical_cores) | std::views::transform([](std::size_t i) { return CpuInfo{i}; });
  }
  // Warning: In contrast to the Linux version, `logical` and `physical` do not share an index set!
  static auto physical_part(std::size_t idx, std::size_t num) {
    const auto physical_cores = *safe_cast<std::size_t>(read_sysctl<int>("hw.physicalcpu"));
    return UniformIndexSegmenter{physical_cores, num}.segment_range(idx) |
           std::views::transform([](std::size_t i) { return CpuInfo{i}; });
  }
};
#elif THES_WINDOWS
struct CpuInfo {
  DWORD id;
  WORD group;
  BYTE logical_processor_index;
  BYTE core_index;
  BYTE numa_node_index;
  BYTE efficiency_class;

  BYTE parked : 1;
  BYTE allocated : 1;
  BYTE allocated_to_target_process : 1;
  BYTE real_time : 1;

  BYTE scheduling_class;
  DWORD64 allocation_tag;

  static std::vector<CpuInfo> logical() {
    // Based on https://github.com/GameTechDev/HybridDetect
    HANDLE current_process = GetCurrentProcess();

    // get the number of bytes in the data structure
    ULONG buffer_size{};
    GetSystemCpuSetInformation(nullptr, 0, &buffer_size, current_process, 0);

    // get all CPU set elements
    auto buffer = std::make_unique<u8[]>(buffer_size);
    const WINBOOL ret = GetSystemCpuSetInformation(
      reinterpret_cast<SYSTEM_CPU_SET_INFORMATION*>(buffer.get()), // NOLINT
      buffer_size, &buffer_size, current_process, 0);
    if (ret != TRUE) {
      throw std::runtime_error{fmt::format("GetSystemCpuSetInformation failed: {}", ret)};
    }

    u8* cpu_set_ptr = buffer.get();
    std::vector<CpuInfo> cores{};

    for (ULONG cpu_set_size = 0; cpu_set_size < buffer_size;) {
      auto* next_cpu_set = reinterpret_cast<SYSTEM_CPU_SET_INFORMATION*>(cpu_set_ptr); // NOLINT

      if (next_cpu_set->Type == CPU_SET_INFORMATION_TYPE::CpuSetInformation) {
        // Store Logical Processor Information for Later Use.
        CpuInfo core{
          .id = next_cpu_set->CpuSet.Id, // NOLINT
          .group = next_cpu_set->CpuSet.Group, // NOLINT
          .logical_processor_index = next_cpu_set->CpuSet.LogicalProcessorIndex, // NOLINT
          .core_index = next_cpu_set->CpuSet.CoreIndex, // NOLINT
          .numa_node_index = next_cpu_set->CpuSet.NumaNodeIndex, // NOLINT
          .efficiency_class = next_cpu_set->CpuSet.EfficiencyClass, // NOLINT
          .parked = next_cpu_set->CpuSet.Parked, // NOLINT
          .allocated = next_cpu_set->CpuSet.Allocated, // NOLINT
          .allocated_to_target_process = next_cpu_set->CpuSet.AllocatedToTargetProcess, // NOLINT
          .real_time = next_cpu_set->CpuSet.RealTime, // NOLINT
          .scheduling_class = next_cpu_set->CpuSet.SchedulingClass, // NOLINT
          .allocation_tag = next_cpu_set->CpuSet.AllocationTag, // NOLINT
        };
        cores.push_back(core);
      }

      cpu_set_ptr += next_cpu_set->Size;
      cpu_set_size += next_cpu_set->Size;
    }

    return cores;
  }

  static std::vector<CpuInfo> logical(EfficiencyClass efficiency_class) {
    using CpuInfos = std::vector<CpuInfo>;
    CpuInfos infos = logical();
    if (efficiency_class == EfficiencyClass::any) {
      return infos;
    }

    ankerl::unordered_dense::set<BYTE> efficiency_set{};
    for (const CpuInfo& info : infos) {
      efficiency_set.emplace(info.efficiency_class);
    }
    if (efficiency_set.empty()) {
      throw std::runtime_error{"There are no efficiency classes (somehow)!"};
    }
    if (efficiency_set.size() > 2) {
      throw std::runtime_error{"There are too many efficiency classes!"};
    }
    std::vector<BYTE> efficiencies = std::move(efficiency_set).extract();
    std::ranges::sort(efficiencies);

    const BYTE selected_class = [&] {
      switch (efficiency_class) {
        case EfficiencyClass::efficiency: return efficiencies.front();
        case EfficiencyClass::performance: return efficiencies.back();
        default: throw std::runtime_error{"Unsupported efficiency class!"};
      }
    }();
    auto range = std::move(infos) | std::views::filter([selected_class](const CpuInfo& info) {
                   return info.efficiency_class == selected_class;
                 });
    return std::ranges::to<CpuInfos>(range);
  }

  static auto physical() {
    return logical() | std::views::filter([](const CpuInfo& info) {
             return info.logical_processor_index == info.core_index;
           });
  }
  static auto physical(EfficiencyClass efficiency_class) {
    return logical(efficiency_class) | std::views::filter([](const CpuInfo& info) {
             return info.logical_processor_index == info.core_index;
           });
  }

  static auto physical_part(std::size_t idx, std::size_t num) {
    auto one_per_core = std::ranges::to<std::vector<CpuInfo>>(CpuInfo::physical());
    const auto subsizes = UniformIndexSegmenter{one_per_core.size(), num}.segment_range(idx);
    return std::move(one_per_core) | std::views::drop(subsizes.begin_value()) |
           std::views::take(subsizes.size());
  }
  static auto physical_part(EfficiencyClass efficiency_class, std::size_t idx, std::size_t num) {
    auto one_per_core = std::ranges::to<std::vector<CpuInfo>>(CpuInfo::physical(efficiency_class));
    const auto subsizes = UniformIndexSegmenter{one_per_core.size(), num}.segment_range(idx);
    return std::move(one_per_core) | std::views::drop(subsizes.begin_value()) |
           std::views::take(subsizes.size());
  }
};
#endif
} // namespace thes

#endif // INCLUDE_THESAUROS_RESOURCES_CPU_INFO_HPP
