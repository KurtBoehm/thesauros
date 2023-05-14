#ifndef INCLUDE_THESAUROS_UTILITY_BYTE_INTEGER_HPP
#define INCLUDE_THESAUROS_UTILITY_BYTE_INTEGER_HPP

#include <bit>
#include <climits>
#include <cstddef>

#include "thesauros/utility/fixed-size-integer.hpp"

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
