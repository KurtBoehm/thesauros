// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_BYTE_INTEGER_HPP
#define INCLUDE_THESAUROS_UTILITY_BYTE_INTEGER_HPP

#include <bit>
#include <climits>
#include <cstddef>
#include <limits>

#include "thesauros/types/fixed-size-integer.hpp"

namespace thes {
template<std::size_t tByteNum>
struct ByteInteger {
  static constexpr std::size_t byte_num = tByteNum;
  static constexpr std::size_t bit_num = CHAR_BIT * byte_num;

  using Unsigned = FixedUnsignedInt<std::bit_ceil(byte_num)>;

  static constexpr std::size_t overhead_byte_num = sizeof(Unsigned) - byte_num;
  static constexpr std::size_t overhead_bit_num = CHAR_BIT * overhead_byte_num;
  static constexpr Unsigned max = std::numeric_limits<Unsigned>::max() >> overhead_bit_num;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_BYTE_INTEGER_HPP
