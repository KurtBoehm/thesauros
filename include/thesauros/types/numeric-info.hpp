// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_TYPES_NUMERIC_INFO_HPP
#define INCLUDE_THESAUROS_TYPES_NUMERIC_INFO_HPP

#include <climits>
#include <limits>

namespace thes {
template<typename T>
struct NumericInfo {
  static constexpr auto byte_num = sizeof(T);
  static constexpr auto bit_num = CHAR_BIT * byte_num;

  static constexpr unsigned digits = static_cast<unsigned>(std::numeric_limits<T>::digits);
  static constexpr T max = std::numeric_limits<T>::max();
};
} // namespace thes

#endif // INCLUDE_THESAUROS_TYPES_NUMERIC_INFO_HPP
