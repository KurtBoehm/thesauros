#ifndef INCLUDE_THESAUROS_MATH_ARITHMETIC_HPP
#define INCLUDE_THESAUROS_MATH_ARITHMETIC_HPP

#include <algorithm>
#include <concepts>
#include <limits>

#include "thesauros/math/overflow.hpp"
#include "thesauros/optimization.hpp"

namespace thes {
template<typename T>
requires std::unsigned_integral<T>
inline constexpr T add_max(T s1, T s2, T maximum) {
  const auto [v, overflow] = overflow_add(s1, s2);
  return overflow ? maximum : std::min(v, maximum);
}

template<typename T>
inline constexpr T div_ceil(const T dividend, const T divisor) {
  return dividend / divisor + T{dividend % divisor != 0};
}

template<typename T>
inline constexpr T prod_div(const T factor1, const T factor2, const T divisor) {
  assert(divisor > 0);
  assert(factor2 <= std::numeric_limits<T>::max() / divisor);

  const T div = factor1 / divisor;
  const T rem = factor1 % divisor;
  return factor2 * div + (factor2 * rem) / divisor;
}

template<typename TBase, typename TUInt>
requires std::unsigned_integral<TUInt>
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

template<typename T>
inline constexpr int log2_floor(const T n) {
  return std::bit_width(n) - 1;
}
template<typename T>
inline constexpr int log2_ceil(const T n) {
  return std::bit_width(n - 1);
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
    const T nbase{-base};
    for (; num <= nbase; ++out, num /= base) {
    }
  } else {
    for (; num >= base; ++out, num /= base) {
    }
  }
  return out + 1;
}

template<typename T>
requires std::unsigned_integral<T>
inline constexpr T set_bit(T value, T bit_index, bool bit_value) {
  return (value & T(~(T{1} << bit_index))) + (T{bit_value} << bit_index);
}

// Computes floor(x^(1/n)) precisely. The complexity is Θ(log2(x)/n).
template<auto tExponent, typename T>
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
