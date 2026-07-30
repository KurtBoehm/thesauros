// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_FORMAT_MULTI_SIZE_HPP
#define INCLUDE_THESAUROS_FORMAT_MULTI_SIZE_HPP

#include <cstddef>

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/format/formatter.hpp"
#include "thesauros/utility/multi-size.hpp"

template<typename S, std::size_t N>
struct fmt::formatter<thes::SubMultiSize<S, N>> : thes::SimpleFormatter<> {
  auto format(const thes::SubMultiSize<S, N>& sms, format_context& ctx) const {
    return this->write_padded(
      ctx, [&](auto out) { return fmt::format_to(out, "{}", fmt::join(sms.axis_ranges(), "×")); });
  }
};

#endif // INCLUDE_THESAUROS_FORMAT_MULTI_SIZE_HPP
