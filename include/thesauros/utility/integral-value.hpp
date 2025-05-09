// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_INTEGRAL_VALUE_HPP
#define INCLUDE_THESAUROS_UTILITY_INTEGRAL_VALUE_HPP

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes {
template<typename T>
struct IntegralValueTrait {
  using Type = T;
  static constexpr T value(const T value) THES_ALWAYS_INLINE {
    return value;
  }
};
template<typename T, T tValue>
struct IntegralValueTrait<ValueTag<T, tValue>> {
  using Type = IntegralValueTrait<T>::Type;
  static constexpr Type value(const ValueTag<T, tValue> tag) THES_ALWAYS_INLINE {
    return IntegralValueTrait<T>::value(tag.value);
  }
};

template<typename T>
using IntegralValue = IntegralValueTrait<T>::Type;
template<typename T>
inline constexpr IntegralValue<T> integral_value(const T value) {
  return IntegralValueTrait<T>::value(value);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_INTEGRAL_VALUE_HPP
