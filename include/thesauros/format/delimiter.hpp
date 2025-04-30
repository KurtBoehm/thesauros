// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_FORMAT_DELIMITER_HPP
#define INCLUDE_THESAUROS_FORMAT_DELIMITER_HPP

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/format/formatter.hpp"
#include "thesauros/io/delimiter.hpp"

template<>
struct fmt::formatter<thes::Delimiter> : public thes::SimpleFormatter<> {
  auto format(const thes::Delimiter& delim, format_context& ctx) const {
    return this->write_padded(ctx, [&](auto it) { return delim.output(it); });
  }
};

#endif // INCLUDE_THESAUROS_FORMAT_DELIMITER_HPP
