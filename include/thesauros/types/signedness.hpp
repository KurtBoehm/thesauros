// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_TYPES_SIGNEDNESS_HPP
#define INCLUDE_THESAUROS_TYPES_SIGNEDNESS_HPP

#include <type_traits>

#include "thesauros/types/primitives.hpp"

namespace thes {
template<typename T>
struct ChangeSignednessTrait {
  using Signed = std::make_signed_t<T>;
  using Unsigned = std::make_unsigned_t<T>;
};
template<>
struct ChangeSignednessTrait<i128> {
  using Signed = i128;
  using Unsigned = u128;
};
template<>
struct ChangeSignednessTrait<u128> {
  using Signed = i128;
  using Unsigned = u128;
};

template<typename T>
using MakeSigned = ChangeSignednessTrait<T>::Signed;
template<typename T>
using MakeUnsigned = ChangeSignednessTrait<T>::Unsigned;
} // namespace thes

#endif // INCLUDE_THESAUROS_TYPES_SIGNEDNESS_HPP
