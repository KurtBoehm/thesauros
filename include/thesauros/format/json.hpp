// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_FORMAT_JSON_HPP
#define INCLUDE_THESAUROS_FORMAT_JSON_HPP

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/format/formatter.hpp"
#include "thesauros/io/json.hpp"

template<typename T>
struct fmt::formatter<thes::JsonPrinter<T>> : public thes::SimpleFormatter<> {
  auto format(thes::JsonPrinter<T> p, format_context& ctx) const {
    return this->write_padded(ctx, [&](auto it) { return write_json(it, p.value(), p.indent()); });
  }
};
template<>
struct fmt::formatter<thes::Indentation> : public thes::SimpleFormatter<> {
  auto format(const thes::Indentation& indent, format_context& ctx) const {
    return this->write_padded(ctx, [&](auto it) { return indent.output(it); });
  }
};

#endif // INCLUDE_THESAUROS_FORMAT_JSON_HPP
