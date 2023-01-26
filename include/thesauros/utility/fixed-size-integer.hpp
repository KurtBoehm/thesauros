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
  using utype = u8;
  using itype = i8;
};
template<>
struct FixedInt<2> {
  using utype = u16;
  using itype = i16;
};
template<>
struct FixedInt<4> {
  using utype = u32;
  using itype = i32;
};
template<>
struct FixedInt<8> {
  using utype = u64;
  using itype = i64;
};
template<>
struct FixedInt<16> {
  using utype = u128;
  using itype = i128;
};
} // namespace utility_impl

template<std::size_t tByteNum>
using FixedUnsignedInt = typename utility_impl::FixedInt<tByteNum>::utype;
template<std::size_t tByteNum>
using FixedSignedInt = typename utility_impl::FixedInt<tByteNum>::itype;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_FIXED_SIZE_INTEGER_HPP
