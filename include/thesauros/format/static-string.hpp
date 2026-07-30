// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_FORMAT_STATIC_STRING_HPP
#define INCLUDE_THESAUROS_FORMAT_STATIC_STRING_HPP

#include <cstddef>
#include <type_traits>

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/string/static-string.hpp"

template<std::size_t N, typename Char>
struct fmt::is_tuple_formattable<thes::StaticString<N>, Char> : public std::false_type {};

#endif // INCLUDE_THESAUROS_FORMAT_STATIC_STRING_HPP
