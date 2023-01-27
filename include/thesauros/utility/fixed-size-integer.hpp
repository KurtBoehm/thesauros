#ifndef INCLUDE_THESAUROS_UTILITY_FIXED_SIZE_INTEGER_HPP
#define INCLUDE_THESAUROS_UTILITY_FIXED_SIZE_INTEGER_HPP

#include <bit>

#include "primitives.hpp"

namespace thes {
namespace utility_impl {
template<std::size_t tByteNum>
struct FixedInt : public FixedInt<std::bit_ceil(tByteNum)> {};

template<>
struct FixedInt<1> {
  using UnsignedInt = U8;
  using SignedInt = I8;
};
template<>
struct FixedInt<2> {
  using UnsignedInt = U16;
  using SignedInt = I16;
};
template<>
struct FixedInt<4> {
  using UnsignedInt = U32;
  using SignedInt = I32;
};
template<>
struct FixedInt<8> {
  using UnsignedInt = U64;
  using SignedInt = I64;
};
template<>
struct FixedInt<16> {
  using UnsignedInt = U128;
  using SignedInt = I128;
};
} // namespace utility_impl

template<std::size_t tByteNum>
using FixedUnsignedInt = typename utility_impl::FixedInt<tByteNum>::UnsignedInt;
template<std::size_t tByteNum>
using FixedSignedInt = typename utility_impl::FixedInt<tByteNum>::SignedInt;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_FIXED_SIZE_INTEGER_HPP
