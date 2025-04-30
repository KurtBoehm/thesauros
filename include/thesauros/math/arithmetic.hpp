// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_MATH_ARITHMETIC_HPP
#define INCLUDE_THESAUROS_MATH_ARITHMETIC_HPP

#include <algorithm>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <limits>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"

namespace thes {
template<std::unsigned_integral T>
inline constexpr T add_max(T a, T b, T max) {
  const T ub = std::numeric_limits<T>::max() - a;
  const T satsum = a + std::min(b, ub);
  return std::min(satsum, max);
}
template<std::unsigned_integral T>
inline constexpr T sub_min(T a, T b, T min) {
  const T satdif = a - std::min(a, b);
  return std::max(satdif, min);
}

template<typename T>
inline constexpr T div_ceil(const T dividend, const T divisor) {
  return static_cast<T>(dividend / divisor + T{dividend % divisor != 0});
}

template<typename TBase, std::unsigned_integral TUInt>
inline constexpr TBase pow(TBase x, const TUInt exponent) {
  const unsigned iter = std::bit_width(exponent);
  TBase y = 1;
  for (unsigned i = 0; i < iter; ++i, x *= x) {
    y *= (exponent & (1U << i)) ? x : 1;
  }
  return y;
}

template<std::size_t tExponent, typename T>
inline constexpr T pow(const T& value) {
  if constexpr (tExponent == 0) {
    return T{1};
  }
  if constexpr (tExponent == 1) {
    return value;
  }

  const T half_exp{pow<tExponent / 2>(value)};
  if constexpr (tExponent % 2 == 0) {
    return half_exp * half_exp;
  } else {
    return half_exp * half_exp * value;
  }
}

inline constexpr unsigned log2_floor(const auto n) {
  assert(n != 0);
  return static_cast<unsigned>(std::bit_width(n) - 1);
}
inline constexpr unsigned log2_ceil(const auto n) {
  assert(n != 0);
  return static_cast<unsigned>(std::bit_width(n - 1));
}

template<typename T>
inline constexpr T bit_mask(T a) {
  const auto w = std::countl_zero(a);
  return (w == std::numeric_limits<T>::digits) ? 0 : (std::numeric_limits<T>::max() >> w);
}

template<typename T>
inline consteval unsigned abs_log_ceil(T base, T num) {
  unsigned out = 0;
  if (num < 0) {
    const auto nbase = static_cast<T>(-base);
    for (; num <= nbase; ++out, num /= base) {
    }
  } else {
    for (; num >= base; ++out, num /= base) {
    }
  }
  return out + 1;
}

template<std::unsigned_integral T>
inline constexpr T set_bit(T value, auto bit_index, bool bit_value) {
  return static_cast<T>((value & T(~(T{1} << bit_index))) + (T{bit_value} << bit_index));
}
template<std::unsigned_integral T>
inline constexpr bool get_bit(T value, auto bit_index) {
  return value & T(T{1} << bit_index);
}
template<std::unsigned_integral T, typename... TArgs>
requires(... && std::same_as<TArgs, bool>)
inline constexpr T combine_bits(TArgs... bits) {
  return [bits...]<std::size_t... tIdxs>(std::index_sequence<tIdxs...> /*idxs*/) {
    T out{};
    (out += ... += T(T{bits} << tIdxs));
    return out;
  }(std::index_sequence_for<TArgs...>{});
}

// Computes floor(x^(1/n)) precisely. The complexity is Θ(log2(x)/n).
template<auto tExponent, std::unsigned_integral T>
THES_ALWAYS_INLINE inline constexpr T int_root(T x) {
  if (x < 2) {
    return x;
  }
  const unsigned bit_num = log2_floor(x) / tExponent;
  T root = T{1} << bit_num;
  T bit = root >> 1;
  for (T i = 0; i < std::numeric_limits<T>::digits; ++i) {
    const T part = root | bit;
    const T sq = pow<tExponent>(part);
    root = (x >= sq) ? part : root;
    bit >>= 1;
    if (bit == 0) {
      break;
    }
  }
  return root;
}

template<typename T>
inline constexpr T greatest_divisor(T value) {
  for (T counter = int_root<2>(value); counter > 1; --counter) {
    if (value % counter == 0) {
      return counter;
    }
  }
  return 1;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_MATH_ARITHMETIC_HPP
