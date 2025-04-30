// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STRING_CHARACTER_TOOLS_HPP
#define INCLUDE_THESAUROS_STRING_CHARACTER_TOOLS_HPP

namespace thes {
inline constexpr bool is_uppercase(char c) {
  return 'A' <= c && c <= 'Z';
}
inline constexpr char to_lowercase(char c) {
  if (is_uppercase(c)) {
    c -= 'A';
    c += 'a';
  }
  return c;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_STRING_CHARACTER_TOOLS_HPP
