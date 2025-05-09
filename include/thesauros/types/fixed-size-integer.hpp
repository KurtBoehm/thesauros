// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_TYPES_FIXED_SIZE_INTEGER_HPP
#define INCLUDE_THESAUROS_TYPES_FIXED_SIZE_INTEGER_HPP

#include <bit>
#include <cstddef>

#include "thesauros/types/primitives.hpp"

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
using FixedUnsignedInt = FixedIntTrait<tByteNum>::Unsigned;
template<std::size_t tByteNum>
using FixedSignedInt = FixedIntTrait<tByteNum>::Signed;
} // namespace thes

#endif // INCLUDE_THESAUROS_TYPES_FIXED_SIZE_INTEGER_HPP
