// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_MATH_COMPILE_TIME_HPP
#define INCLUDE_THESAUROS_MATH_COMPILE_TIME_HPP

#include <cmath>
#include <concepts>
#include <cstddef>
#include <limits>

// This is somewhat inspired by https://github.com/kthohr/gcem, but the implementation
// differs significantly

namespace thes::ctm {
// This function is not guaranteed to transform -1 to 1
template<typename T>
constexpr T abs(T value) {
  if consteval {
    return value < 0 ? -value : value;
  } else {
    return std::abs(value);
  }
}

template<std::floating_point T>
constexpr bool is_nan(const T x) noexcept {
  return x != x;
}
template<std::floating_point T>
constexpr bool is_posinf(const T x) noexcept {
  return x == std::numeric_limits<T>::infinity();
}

template<std::floating_point T>
constexpr T sqrt(const T x) noexcept {
  if (is_nan(x) || x < T{0}) {
    return std::numeric_limits<T>::quiet_NaN();
  }
  if (is_posinf(x) || ctm::abs(T{1} - x) < std::numeric_limits<T>::min()) {
    return x;
  }
  if (ctm::abs(x) < std::numeric_limits<T>::min()) {
    return T{0};
  }
  // Implement Heron’s method with a fancy stopping criterion courtesy of GCEM
  T xn = x;
  for (std::size_t i = 0; i < 128; ++i) {
    if (ctm::abs(xn - x / xn) / (T{1} + xn) < std::numeric_limits<T>::min()) {
      break;
    }
    xn = T{0.5} * (xn + x / xn);
  }
  return xn;
}
} // namespace thes::ctm

#endif // INCLUDE_THESAUROS_MATH_COMPILE_TIME_HPP
