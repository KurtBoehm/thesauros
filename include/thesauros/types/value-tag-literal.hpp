// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_TYPES_VALUE_TAG_LITERAL_HPP
#define INCLUDE_THESAUROS_TYPES_VALUE_TAG_LITERAL_HPP

#include <cstddef>

#include "thesauros/charconv/parse-integer.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes {
struct IndexTagLiteral {
  constexpr IndexTagLiteral(const char* charray)
      : value(parse_integer<std::size_t>(charray).value()) {}

  std::size_t value;
};

namespace literals {
template<IndexTagLiteral tString>
constexpr auto operator""_it() {
  return thes::index_tag<tString.value>;
}
} // namespace literals
} // namespace thes

#endif // INCLUDE_THESAUROS_TYPES_VALUE_TAG_LITERAL_HPP
