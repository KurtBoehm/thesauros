// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_FORMAT_STATIC_STRING_HPP
#define INCLUDE_THESAUROS_FORMAT_STATIC_STRING_HPP

#include <cstddef>
#include <string_view>

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/string/static-string.hpp"

template<std::size_t tSize>
struct fmt::formatter<thes::StaticString<tSize>> : fmt::nested_formatter<std::string_view> {
  auto format(const thes::StaticString<tSize>& str, format_context& ctx) const {
    return this->write_padded(
      ctx, [&](auto out) { return fmt::format_to(out, "{}", this->nested(str.view())); });
  }
};

#endif // INCLUDE_THESAUROS_FORMAT_STATIC_STRING_HPP
