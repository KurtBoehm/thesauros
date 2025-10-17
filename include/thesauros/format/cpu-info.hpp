// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_FORMAT_CPU_INFO_HPP
#define INCLUDE_THESAUROS_FORMAT_CPU_INFO_HPP

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/macropolis/platform.hpp"
#include "thesauros/resources/cpu-info.hpp"

#if THES_LINUX || THES_APPLE
#include <cstddef>
#elif THES_WINDOWS
#include "thesauros/format/formatter.hpp"
#endif

#if THES_LINUX || THES_APPLE
template<>
struct fmt::formatter<thes::CpuInfo> : public fmt::nested_formatter<std::size_t> {
  auto format(const thes::CpuInfo& info, fmt::format_context& ctx) const {
    return this->write_padded(
      ctx, [&](auto it) { return fmt::format_to(it, "cpu{}", this->nested(info.id)); });
  }
};
#else
template<>
struct fmt::formatter<thes::CpuInfo> : public thes::SimpleFormatter<> {
  auto format(const thes::CpuInfo& info, fmt::format_context& ctx) const {
    return this->write_padded(ctx, [&](auto it) {
      return fmt::format_to(it,
                            "CpuInfo{{id={}, group={}, logical_processor_index={}, "
                            "core_index={}, numa_node_index={}, efficiency_class={}}}",
                            info.id, info.group, info.logical_processor_index, info.core_index,
                            info.numa_node_index, info.efficiency_class);
    });
  }
};
#endif

#endif // INCLUDE_THESAUROS_FORMAT_CPU_INFO_HPP
