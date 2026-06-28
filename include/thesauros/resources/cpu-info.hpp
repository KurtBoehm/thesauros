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
/** Describes relative efficiency/performance classes of CPUs. */
enum struct EfficiencyClass : u8 { any, efficiency, medium, performance };

#if THES_LINUX
//--------------------------------------------------------------------------------------------------
// Linux helpers
//--------------------------------------------------------------------------------------------------

/** A view over the CPU indices described by a sysfs range string. */
template<typename TChars>
auto cpu_range(TChars&& full) {
  const std::string_view fullsv{full.data(), full.size()};
  const auto size = full.size() - std::size_t(fullsv.ends_with("\n"));
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

/** Describes a Linux logical CPU and exposes helpers to query topology information. */
struct CpuInfo {
  static constexpr auto cpu_path_cstr = "/sys/devices/system/cpu/";
  static constexpr auto cpu_atom_path_cstr = "/sys/devices/cpu_atom/cpus";
  static constexpr auto cpu_core_path_cstr = "/sys/devices/cpu_core/cpus";

  static constexpr auto physical_filter =
    std::views::filter([](const auto& cpu) { return cpu.id == std::ranges::min(cpu.core_cpus()); });

  std::size_t id;

  /** The sysfs topology directory for this CPU. */
  [[nodiscard]] auto sys_folder() const {
    return std::filesystem::path{cpu_path_cstr} / fmt::format("cpu{}", id) / "topology";
  }

  /** A view over logical CPUs that belong to the same core. */
  [[nodiscard]] auto core_cpus() const {
    return cpu_range(read_file<DynamicArray<char>>(sys_folder() / "core_cpus_list"));
  }

  /** A view over all logical CPUs. */
  static auto logical() {
    const auto present_path = std::filesystem::path{cpu_path_cstr} / "present";
    return cpu_range(read_file<DynamicArray<char>>(present_path)) |
           std::views::transform([](std::size_t i) { return CpuInfo{.id = i}; });
  }

  /** A view over logical CPUs filtered by efficiency class if available. */
  static auto logical(EfficiencyClass efficiency_class) {
    const auto cores_path = [&] {
      switch (efficiency_class) {
        case EfficiencyClass::any: {
          return std::filesystem::path{cpu_path_cstr} / "present";
        }
        case EfficiencyClass::efficiency: {
          const std::filesystem::path atom_path{cpu_atom_path_cstr};
          return std::filesystem::exists(atom_path)
                   ? atom_path
                   : (std::filesystem::path{cpu_path_cstr} / "present");
        }
        case EfficiencyClass::performance: {
          const std::filesystem::path core_path{cpu_core_path_cstr};
          return std::filesystem::exists(core_path)
                   ? core_path
                   : (std::filesystem::path{cpu_path_cstr} / "present");
        }
        default: {
          throw std::invalid_argument{"Unsupported efficiency class!"};
        }
      }
    }();
    return cpu_range(read_file<DynamicArray<char>>(cores_path)) |
           std::views::transform([](std::size_t i) { return CpuInfo{.id = i}; });
  }

  /** A view over physical CPUs (one `CpuInfo` per core). */
  static auto physical() {
    return logical() | physical_filter;
  }

  /** A view over physical CPUs of a given efficiency class. */
  static auto physical(EfficiencyClass efficiency_class) {
    return logical(efficiency_class) | physical_filter;
  }

  /** The number of logical CPUs. */
  static std::size_t num_logical() {
    return std::ranges::distance(logical());
  }

  /** The number of logical CPUs of a given efficiency class. */
  static std::size_t num_logical(EfficiencyClass efficiency_class) {
    return std::ranges::distance(logical(efficiency_class));
  }

  /** The number of physical CPUs. */
  static std::size_t num_physical() {
    return std::ranges::distance(physical());
  }

  /** The number of physical CPUs of a given efficiency class. */
  static std::size_t num_physical(EfficiencyClass efficiency_class) {
    return std::ranges::distance(physical(efficiency_class));
  }

  /** A contiguous subset of CPU infos distributed uniformly across physical CPUs. */
  template<typename CpuInfoRange>
  static auto cpu_infos_part(CpuInfoRange&& cpu_infos, std::size_t idx, std::size_t num) {
    auto one_per_core =
      std::ranges::to<std::vector<CpuInfo>>(std::forward<CpuInfoRange>(cpu_infos));
    const auto subsizes = UniformIndexSegmenter{one_per_core.size(), num}.segment_range(idx);
    return std::move(one_per_core) | std::views::drop(subsizes.begin_value()) |
           std::views::take(subsizes.size());
  }

  /** A partition of the physical CPUs. */
  static auto physical_part(std::size_t idx, std::size_t num) {
    return cpu_infos_part(physical(), idx, num);
  }

  /** A partition of the physical CPUs of the given efficiency class. */
  static auto physical_part(EfficiencyClass efficiency_class, std::size_t idx, std::size_t num) {
    return cpu_infos_part(physical(efficiency_class), idx, num);
  }
};
#elif THES_APPLE
#ifndef THES_USE_IOKIT
#define THES_USE_IOKIT false
#endif

namespace detail {
//--------------------------------------------------------------------------------------------------
// Apple helpers
//--------------------------------------------------------------------------------------------------

/** Read a sysctl value or throw on failure. */
template<typename T>
inline T read_sysctl(const char* name) {
  T value{};
  std::size_t size = sizeof(T);
  const int ret = sysctlbyname(name, &value, &size, nullptr, 0);
  if (ret != 0) {
    throw std::runtime_error{fmt::format("sysctlbyname({}) failed: {}", name, ret)};
  }
  return value;
}

/** Query a sysctl value and return `std::nullopt` on failure. */
template<typename T>
inline std::optional<T> query_sysctl(const char* name) {
  T value{};
  std::size_t size = sizeof(T);
  const int ret = sysctlbyname(name, &value, &size, nullptr, 0);
  if (ret != 0) {
    return std::nullopt;
  }
  return value;
}

/** Captures per-performance-level CPU counts on Apple systems. */
struct PerfLevel {
  /** `perflevelN` index. */
  int index;
  /** `hw.perflevelN.name`. */
  std::string name;
  /** Mapped to our efficiency class. */
  EfficiencyClass eff_class = EfficiencyClass::any;
  /** `hw.perflevelN.physicalcpu`. */
  int physical_cpus;
  /** `hw.perflevelN.logicalcpu`. */
  int logical_cpus;
};

/** Read performance levels and map them to efficiency classes. */
inline std::vector<PerfLevel> read_perflevels() {
  std::vector<PerfLevel> levels{};

  // As of 2026, none of Apple’s CPUs has more than two performance levels.
  for (int i = 0; i < 4; ++i) {
    const auto base = fmt::format("hw.perflevel{}", i);

    const auto name_key = fmt::format("{}.name", base);
    const auto name_arr = query_sysctl<std::array<char, 128>>(name_key.c_str());
    if (!name_arr.has_value()) {
      break;
    }
    std::string name{name_arr->data(), strnlen(name_arr->data(), name_arr->size())};

    const auto phys_key = base + ".physicalcpu";
    const auto phys = read_sysctl<int>(phys_key.c_str());

    const auto log_key = base + ".logicalcpu";
    const auto log = read_sysctl<int>(log_key.c_str());

    // The efficiency class will be determined later.
    levels.push_back(PerfLevel{
      .index = i,
      .name = std::move(name),
      .physical_cpus = phys,
      .logical_cpus = log,
    });
  }

  if (levels.empty()) {
    throw std::runtime_error{"No performance levels found through sysctl"};
  }

  // Check whether any core is a “Super” core:
  //
  // - “Super” exists: “Super” → `performance`, “Performance” → `medium`.
  // - “Super” does not exist: “Performance” → `performance`.
  //
  // “Efficiency” is always mapped to `efficiency`.
  const bool any_super =
    std::ranges::any_of(levels, [](const PerfLevel& level) { return level.name == "Super"; });
  const EfficiencyClass perf_class =
    any_super ? EfficiencyClass::medium : EfficiencyClass::performance;

  for (auto& level : levels) {
    if (level.name == "Super") {
      level.eff_class = EfficiencyClass::performance;
    } else if (level.name == "Performance") {
      level.eff_class = perf_class;
    } else if (level.name == "Efficiency") {
      level.eff_class = EfficiencyClass::efficiency;
    } else {
      throw std::runtime_error{"Unsupported efficiency class " + level.name};
    }
  }

  // Sort by ascending performance level, which is what IOKit seems to do.
  std::ranges::sort(levels, {}, &PerfLevel::eff_class);

  return levels;
}

#if THES_USE_IOKIT
//--------------------------------------------------------------------------------------------------
// macOS CPU topology discovery via IORegistry
//--------------------------------------------------------------------------------------------------

/** One logical CPU entry discovered via IOKit. */
struct CpuEntry {
  std::size_t logical_id;
  EfficiencyClass efficiency_class;
};

/** Build the IOKit-based CPU topology. */
std::vector<CpuEntry> compute_cpu_topology();

/** A cached IOKit-based CPU topology. */
inline const std::vector<CpuEntry>& apple_cpu_topology() {
  static const std::vector<CpuEntry> cache = compute_cpu_topology();
  return cache;
}
#endif
} // namespace detail

/** Describes a logical CPU on Apple platforms. */
struct CpuInfo {
  std::size_t id;
  EfficiencyClass efficiency_class = EfficiencyClass::any;

  /** All logical CPUs. */
  static std::vector<CpuInfo> logical() {
#if THES_USE_IOKIT
    return std::ranges::to<std::vector<CpuInfo>>(
      detail::apple_cpu_topology() | std::views::transform([](detail::CpuEntry entry) {
        return CpuInfo{.id = entry.logical_id, .efficiency_class = entry.efficiency_class};
      }));
#endif

    const auto levels = detail::read_perflevels();
    const auto logicalcpu = *safe_cast<std::size_t>(detail::read_sysctl<int>("hw.logicalcpu"));

    const auto sum_levels =
      std::ranges::fold_left(levels | std::views::transform([](const detail::PerfLevel& level) {
                               return *safe_cast<std::size_t>(level.logical_cpus);
                             }),
                             0UZ, std::plus{});
    if (sum_levels != logicalcpu) {
      throw std::runtime_error{"Inconsistent CPU count"};
    }

    std::vector<CpuInfo> infos{};
    infos.reserve(logicalcpu);

    std::size_t current_id = 0;
    for (const auto& lvl : levels) {
      for (int i = 0; i < lvl.logical_cpus && current_id < logicalcpu; ++i) {
        infos.push_back({.id = current_id++, .efficiency_class = lvl.eff_class});
      }
    }

    return infos;
  }

  /** All logical CPUs with the given efficiency class. */
  static std::vector<CpuInfo> logical(EfficiencyClass efficiency_class) {
    if (efficiency_class == EfficiencyClass::any) {
      return logical();
    }
    return std::ranges::to<std::vector<CpuInfo>>(logical() |
                                                 std::views::filter([&](const CpuInfo& info) {
                                                   return info.efficiency_class == efficiency_class;
                                                 }));
  }

  /** All physical CPUs; must match the logical count on Apple Silicon. */
  static std::vector<CpuInfo> physical() {
    const auto logicalcpu = *safe_cast<std::size_t>(detail::read_sysctl<int>("hw.logicalcpu"));
    const auto physicalcpu = *safe_cast<std::size_t>(detail::read_sysctl<int>("hw.physicalcpu"));
    if (logicalcpu != physicalcpu) {
      throw std::runtime_error{"Number of logical and physical CPUs does not match;"
                               "Apple Silicon does not use SMT and that is all that we support"};
    }
    return logical();
  }

  /** All physical CPUs with the given efficiency class. */
  static std::vector<CpuInfo> physical(EfficiencyClass efficiency_class) {
    const auto logicalcpu = *safe_cast<std::size_t>(detail::read_sysctl<int>("hw.logicalcpu"));
    const auto physicalcpu = *safe_cast<std::size_t>(detail::read_sysctl<int>("hw.physicalcpu"));
    if (logicalcpu != physicalcpu) {
      throw std::runtime_error{"Number of logical and physical CPUs does not match;"
                               "Apple Silicon does not use SMT and that is all that we support"};
    }
    return logical(efficiency_class);
  }

  /** A partition of the physical CPUs. */
  static auto physical_part(std::size_t idx, std::size_t num) {
    auto one_per_core = physical();
    const auto subsizes = UniformIndexSegmenter{one_per_core.size(), num}.segment_range(idx);
    return std::move(one_per_core) | std::views::drop(subsizes.begin_value()) |
           std::views::take(subsizes.size());
  }

  /** A partition of the physical CPUs of the given efficiency class. */
  static auto physical_part(EfficiencyClass efficiency_class, std::size_t idx, std::size_t num) {
    auto one_per_core = physical(efficiency_class);
    const auto subsizes = UniformIndexSegmenter{one_per_core.size(), num}.segment_range(idx);
    return std::move(one_per_core) | std::views::drop(subsizes.begin_value()) |
           std::views::take(subsizes.size());
  }

  /** The number of logical CPUs. */
  static std::size_t num_logical() {
    return *safe_cast<std::size_t>(detail::read_sysctl<int>("hw.logicalcpu"));
  }

  /** The number of logical CPUs of a given efficiency class. */
  static std::size_t num_logical(EfficiencyClass efficiency_class) {
    if (efficiency_class == EfficiencyClass::any) {
      return num_logical();
    }

#if THES_USE_IOKIT
    const auto& topo = detail::apple_cpu_topology();
    return std::ranges::count_if(topo, [efficiency_class](const detail::CpuEntry& entry) {
      return entry.efficiency_class == efficiency_class;
    });
#else
    const auto levels = detail::read_perflevels();
    return std::ranges::fold_left(
      levels | std::views::filter([efficiency_class](const detail::PerfLevel& level) {
        return level.eff_class == efficiency_class;
      }) |
        std::views::transform([](const detail::PerfLevel& level) {
          return *safe_cast<std::size_t>(level.logical_cpus);
        }),
      0UZ, std::plus{});
#endif
  }

  /** The number of physical CPUs. */
  static std::size_t num_physical() {
    const auto logicalcpu = *safe_cast<std::size_t>(detail::read_sysctl<int>("hw.logicalcpu"));
    const auto physicalcpu = *safe_cast<std::size_t>(detail::read_sysctl<int>("hw.physicalcpu"));
    if (logicalcpu != physicalcpu) {
      throw std::runtime_error{"Number of logical and physical CPUs does not match;"
                               "Apple Silicon does not use SMT and that is all that we support"};
    }
    return physicalcpu;
  }

  /** The number of physical CPUs of a given efficiency class. */
  static std::size_t num_physical(EfficiencyClass efficiency_class) {
    const auto physicalcpu = num_physical();
    if (efficiency_class == EfficiencyClass::any) {
      return physicalcpu;
    }

#if THES_USE_IOKIT
    // IOKit topology carries no SMT information, so logical count per class is also physical.
    return num_logical(efficiency_class);
#else
    const auto levels = detail::read_perflevels();
    const auto total_physical =
      std::ranges::fold_left(levels | std::views::transform([](const detail::PerfLevel& level) {
                               return *safe_cast<std::size_t>(level.physical_cpus);
                             }),
                             0UZ, std::plus{});
    if (total_physical != physicalcpu) {
      throw std::runtime_error{"Inconsistent physical CPU count"};
    }

    return std::ranges::fold_left(
      levels | std::views::filter([efficiency_class](const detail::PerfLevel& level) {
        return level.eff_class == efficiency_class;
      }) |
        std::views::transform([](const detail::PerfLevel& level) {
          return *safe_cast<std::size_t>(level.physical_cpus);
        }),
      0UZ, std::plus{});
#endif
  }
};
#elif THES_WINDOWS
/** Describes a logical CPU and its scheduling properties on Windows. */
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

  /** All logical CPUs visible to the current process. */
  static std::vector<CpuInfo> logical() {
    // Based on https://github.com/GameTechDev/HybridDetect
    HANDLE current_process = GetCurrentProcess();

    // The number of bytes in the data structure.
    ULONG buffer_size{};
    GetSystemCpuSetInformation(nullptr, 0, &buffer_size, current_process, 0);

    // All CPU set elements.
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
        // Store logical processor information for later use.
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

  /** All logical CPUs, filtered by efficiency class if requested. */
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
        case EfficiencyClass::efficiency: {
          return efficiencies.front();
        }
        case EfficiencyClass::performance: {
          return efficiencies.back();
        }
        default: {
          throw std::runtime_error{"Unsupported efficiency class!"};
        }
      }
    }();
    auto range = std::move(infos) | std::views::filter([selected_class](const CpuInfo& info) {
                   return info.efficiency_class == selected_class;
                 });
    return std::ranges::to<CpuInfos>(range);
  }

  /** A view over one logical CPU per core. */
  static auto physical() {
    return logical() | std::views::filter([](const CpuInfo& info) {
             return info.logical_processor_index == info.core_index;
           });
  }

  /** A view over one logical CPU per core for the given efficiency class. */
  static auto physical(EfficiencyClass efficiency_class) {
    return logical(efficiency_class) | std::views::filter([](const CpuInfo& info) {
             return info.logical_processor_index == info.core_index;
           });
  }

  /** A partition of the physical CPUs. */
  static auto physical_part(std::size_t idx, std::size_t num) {
    auto one_per_core = std::ranges::to<std::vector<CpuInfo>>(CpuInfo::physical());
    const auto subsizes = UniformIndexSegmenter{one_per_core.size(), num}.segment_range(idx);
    return std::move(one_per_core) | std::views::drop(subsizes.begin_value()) |
           std::views::take(subsizes.size());
  }

  /** A partition of the physical CPUs for the given efficiency class. */
  static auto physical_part(EfficiencyClass efficiency_class, std::size_t idx, std::size_t num) {
    auto one_per_core = std::ranges::to<std::vector<CpuInfo>>(CpuInfo::physical(efficiency_class));
    const auto subsizes = UniformIndexSegmenter{one_per_core.size(), num}.segment_range(idx);
    return std::move(one_per_core) | std::views::drop(subsizes.begin_value()) |
           std::views::take(subsizes.size());
  }

  /** The number of logical CPUs. */
  static std::size_t num_logical() {
    return std::ranges::size(logical());
  }

  /** The number of logical CPUs of a given efficiency class. */
  static std::size_t num_logical(EfficiencyClass efficiency_class) {
    return std::ranges::size(logical(efficiency_class));
  }

  /** The number of physical CPUs. */
  static std::size_t num_physical() {
    return std::ranges::distance(physical());
  }

  /** The number of physical CPUs of a given efficiency class. */
  static std::size_t num_physical(EfficiencyClass efficiency_class) {
    return std::ranges::distance(physical(efficiency_class));
  }
};
#endif
} // namespace thes

#endif // INCLUDE_THESAUROS_RESOURCES_CPU_INFO_HPP
