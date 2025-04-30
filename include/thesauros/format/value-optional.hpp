// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_FORMAT_VALUE_OPTIONAL_HPP
#define INCLUDE_THESAUROS_FORMAT_VALUE_OPTIONAL_HPP

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/utility/value-optional.hpp"

template<typename T, T tEmpty>
struct fmt::formatter<thes::ValueOptional<T, tEmpty>> : fmt::nested_formatter<T> {
  auto format(thes::ValueOptional<T, tEmpty> opt, format_context& ctx) const {
    if (opt.has_value()) {
      return this->write_padded(
        ctx, [&](auto out) { return fmt::format_to(out, "{}", this->nested(*opt)); });
    }
    return this->write_padded(ctx, [&](auto out) { return fmt::format_to(out, "empty"); });
  }
};

#endif // INCLUDE_THESAUROS_FORMAT_VALUE_OPTIONAL_HPP
