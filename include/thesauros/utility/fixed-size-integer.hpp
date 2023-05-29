#ifndef INCLUDE_THESAUROS_UTILITY_FIXED_SIZE_INTEGER_HPP
#define INCLUDE_THESAUROS_UTILITY_FIXED_SIZE_INTEGER_HPP

#include <bit>
#include <cstddef>

#include "primitives.hpp"

namespace thes {
template<std::size_t tByteNum>
struct FixedIntTrait : public FixedIntTrait<std::bit_ceil(tByteNum)> {};

template<>
struct FixedIntTrait<1> {
  using Unsigned = u8;
  using Signed = i8;
};
template<>
struct FixedIntTrait<2> {
  using Unsigned = u16;
  using Signed = i16;
};
template<>
struct FixedIntTrait<4> {
  using Unsigned = u32;
  using Signed = i32;
};
template<>
struct FixedIntTrait<8> {
  using Unsigned = u64;
  using Signed = i64;
};
template<>
struct FixedIntTrait<16> {
  using Unsigned = u128;
  using Signed = i128;
};

template<std::size_t tByteNum>
using FixedUnsignedInt = typename FixedIntTrait<tByteNum>::Unsigned;
template<std::size_t tByteNum>
using FixedSignedInt = typename FixedIntTrait<tByteNum>::Signed;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_FIXED_SIZE_INTEGER_HPP
