#ifndef INCLUDE_THESAUROS_MATH_OVERFLOW_HPP
#define INCLUDE_THESAUROS_MATH_OVERFLOW_HPP

#include <concepts>
#include <limits>

#include "thesauros/utility/info-result.hpp"

namespace thes {
template<typename T>
using OverflowResult = InfoResult<T, bool, false>;

template<typename T>
inline constexpr OverflowResult<T> overflow_add(T arg1, T arg2) {
  T result;
  const bool overflow = __builtin_add_overflow(arg1, arg2, &result);
  return {result, overflow};
}

template<typename T>
inline constexpr OverflowResult<T> overflow_subtract(T arg1, T arg2) {
  T result;
  const bool overflow = __builtin_sub_overflow(arg1, arg2, &result);
  return {result, overflow};
}

template<typename T>
inline constexpr OverflowResult<T> overflow_multiply(T arg1, T arg2) {
  T result;
  const bool overflow = __builtin_mul_overflow(arg1, arg2, &result);
  return {result, overflow};
}

template<typename T = void>
struct OverflowPlus {
  constexpr OverflowResult<T> operator()(T t1, T t2) const {
    return overflow_add(t1, t2);
  }
};
template<>
struct OverflowPlus<void> {
  constexpr auto operator()(auto t1, auto t2) const {
    return overflow_add(t1, t2);
  }
};

template<typename T = void>
struct OverflowMinus {
  constexpr OverflowResult<T> operator()(T t1, T t2) const {
    return overflow_subtract(t1, t2);
  }
};
template<>
struct OverflowMinus<void> {
  constexpr auto operator()(auto t1, auto t2) const {
    return overflow_subtract(t1, t2);
  }
};

template<typename T = void>
struct OverflowMultiplies {
  constexpr OverflowResult<T> operator()(T t1, T t2) const {
    return overflow_multiply(t1, t2);
  }
};
template<>
struct OverflowMultiplies<void> {
  constexpr auto operator()(auto t1, auto t2) const {
    return overflow_multiply(t1, t2);
  }
};

template<std::unsigned_integral T>
inline constexpr T saturate_add(T a, T b) {
  const T sum = a + b;
  return (sum < a) ? std::numeric_limits<T>::max() : sum;
}

template<std::unsigned_integral T>
inline constexpr T saturate_subtract(T a, T b) {
  const T diff = a - b;
  return (a < diff) ? 0 : diff;
}

template<std::unsigned_integral T>
inline constexpr T saturate_multiply(T arg1, T arg2) {
  return overflow_multiply(arg1, arg2).value_or(std::numeric_limits<T>::max());
}
} // namespace thes

#endif // INCLUDE_THESAUROS_MATH_OVERFLOW_HPP
