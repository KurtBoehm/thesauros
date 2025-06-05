// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_RESOURCES_CPU_INFO_HPP
#define INCLUDE_THESAUROS_RESOURCES_CPU_INFO_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <ranges>
#include <string_view>
#include <utility>
#include <vector>

#include "thesauros/charconv/string-convert.hpp"
#include "thesauros/containers/array.hpp"
#include "thesauros/format/fmtlib.hpp"
#include "thesauros/format/formatter.hpp"
#include "thesauros/io/file-reader.hpp"
#include "thesauros/ranges/iota.hpp"
#include "thesauros/utility/index-segmentation.hpp"

namespace thes {
template<typename TChars>
auto cpu_range(TChars&& full) {
  const std::string_view sv{full.data(), full.size()};
  const auto size = full.size() - size_t(sv.ends_with("\n"));
  return std::forward<TChars>(full) | std::views::take(size) | std::views::split(',') |
         std::views::transform([](auto substr) {
           std::string_view sv{substr.data(), substr.size()};
           const std::size_t hyphen = sv.find('-');
           const auto first = string_to_integral<std::size_t>(sv.substr(0, hyphen)).value();
           if (hyphen == std::string_view::npos) {
             return range(first, first + 1);
           }
           assert(sv.find('-', hyphen + 1) == std::string_view::npos);
           const auto second = string_to_integral<std::size_t>(sv.substr(hyphen + 1)).value();
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

  static auto present() {
    const auto present_path = std::filesystem::path{cpu_path_cstr} / "present";
    return cpu_range(read_file<DynamicArray<char>>(present_path)) |
           std::views::transform([](auto i) { return CpuInfo{i}; });
  }

  static auto core_unique() {
    return present() | std::views::filter(
                         [&](auto cpu) { return cpu.id == std::ranges::min(cpu.core_cpus()); });
  }

  static auto core_part(std::size_t idx, std::size_t num) {
    auto common = std::ranges::common_view(CpuInfo::core_unique());
    std::vector<CpuInfo> one_per_core(common.begin(), common.end());
    const auto subsizes = UniformIndexSegmenter{one_per_core.size(), num}.segment_range(idx);
    return std::move(one_per_core) | std::views::drop(subsizes.begin_value()) |
           std::views::take(subsizes.size());
  }
};
} // namespace thes

template<>
struct fmt::formatter<thes::CpuInfo> : public thes::NestedFormatter<std::size_t> {
  auto format(const thes::CpuInfo& info, fmt::format_context& ctx) const {
    return this->write_padded(
      ctx, [&](auto it) { return fmt::format_to(it, "cpu{}", this->nested(info.id)); });
  }
};

#endif // INCLUDE_THESAUROS_RESOURCES_CPU_INFO_HPP
