// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_FORMAT_STRING_ESCAPE_HPP
#define INCLUDE_THESAUROS_FORMAT_STRING_ESCAPE_HPP

#include "thesauros/charconv/string-escape.hpp"
#include "thesauros/format/fmtlib.hpp"
#include "thesauros/format/formatter.hpp"

template<typename T>
struct fmt::formatter<thes::EscapedPrinter<T>> : public thes::SimpleFormatter<> {
  auto format(thes::EscapedPrinter<T> p, format_context& ctx) const {
    return this->write_padded(ctx, [&](auto it) { return thes::escape_string(p.value(), it); });
  }
};

#endif // INCLUDE_THESAUROS_FORMAT_STRING_ESCAPE_HPP
