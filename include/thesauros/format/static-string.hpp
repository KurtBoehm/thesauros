#ifndef INCLUDE_THESAUROS_FORMAT_STATIC_STRING_HPP
#define INCLUDE_THESAUROS_FORMAT_STATIC_STRING_HPP

#include <cstddef>
#include <string_view>

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/utility/static-string.hpp"

template<std::size_t tSize>
struct fmt::formatter<thes::StaticString<tSize>> : fmt::nested_formatter<std::string_view> {
  auto format(const thes::StaticString<tSize>& str, format_context& ctx) const {
    return this->write_padded(
      ctx, [&](auto out) { return fmt::format_to(out, "{}", this->nested(str.view())); });
  }
};

#endif // INCLUDE_THESAUROS_FORMAT_STATIC_STRING_HPP
