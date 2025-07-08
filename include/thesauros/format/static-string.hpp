// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_FORMAT_STATIC_STRING_HPP
#define INCLUDE_THESAUROS_FORMAT_STATIC_STRING_HPP

#include <cstddef>
#include <type_traits>

#include "fmt/ranges.h"

#include "thesauros/string/static-string.hpp"

template<std::size_t tSize, typename TChar>
struct fmt::is_tuple_formattable<thes::StaticString<tSize>, TChar> : public std::false_type {};

#endif // INCLUDE_THESAUROS_FORMAT_STATIC_STRING_HPP
