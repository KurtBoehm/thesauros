#ifndef INCLUDE_THESAUROS_FORMAT_EXECUTION_HPP
#define INCLUDE_THESAUROS_FORMAT_EXECUTION_HPP

#include <cstddef>

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/format/formatter.hpp"
#include "thesauros/resources/cpu-info.hpp"

template<>
struct fmt::formatter<thes::CpuInfo> : public thes::NestedFormatter<std::size_t> {
  auto format(const thes::CpuInfo& info, fmt::format_context& ctx) const {
    return this->write_padded(
      ctx, [&](auto it) { return fmt::format_to(it, "cpu{}", this->nested(info.id)); });
  }
};

#endif // INCLUDE_THESAUROS_FORMAT_EXECUTION_HPP
