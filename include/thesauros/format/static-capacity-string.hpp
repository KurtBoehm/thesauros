// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_FORMAT_STATIC_CAPACITY_STRING_HPP
#define INCLUDE_THESAUROS_FORMAT_STATIC_CAPACITY_STRING_HPP

#include <cstddef>

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/string/static-capacity-string.hpp"

template<std::size_t tCapacity>
struct fmt::range_format_kind<thes::StaticCapacityString<tCapacity>, char> {
  static constexpr auto value = fmt::range_format::disabled;
};

#endif // INCLUDE_THESAUROS_FORMAT_STATIC_CAPACITY_STRING_HPP
