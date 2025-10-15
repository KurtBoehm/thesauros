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
#include <utility>
#include <vector>

#include "thesauros/containers/array.hpp"
#include "thesauros/io/file-reader.hpp"
#elif THES_APPLE
#include <stdexcept>

#include <sys/sysctl.h>
#endif

namespace thes {
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
    return present() | std::views::filter([&](const CpuInfo& cpu) {
             return cpu.id == std::ranges::min(cpu.core_cpus());
           });
  }

  static auto physical_part(std::size_t idx, std::size_t num) {
    auto common = std::ranges::common_view(CpuInfo::physical());
    std::vector<CpuInfo> one_per_core(common.begin(), common.end());
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
#endif
} // namespace thes

#endif // INCLUDE_THESAUROS_RESOURCES_CPU_INFO_HPP
