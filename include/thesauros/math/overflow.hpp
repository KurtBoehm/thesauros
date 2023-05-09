#ifndef INCLUDE_THESAUROS_MATH_OVERFLOW_HPP
#define INCLUDE_THESAUROS_MATH_OVERFLOW_HPP

#include <optional>

namespace thes {
template<typename T>
inline constexpr std::optional<T> overflow_add(T arg1, T arg2) {
  T result;
  if (__builtin_add_overflow(arg1, arg2, &result)) {
    return std::nullopt;
  }
  return result;
}

template<typename T>
inline constexpr std::optional<T> overflow_subtract(T arg1, T arg2) {
  T result;
  if (__builtin_sub_overflow(arg1, arg2, &result)) {
    return std::nullopt;
  }
  return result;
}

template<typename T>
inline constexpr std::optional<T> overflow_multiply(T arg1, T arg2) {
  T result;
  if (__builtin_mul_overflow(arg1, arg2, &result)) {
    return std::nullopt;
  }
  return result;
}

template<typename T = void>
struct OverflowPlus {
  constexpr std::optional<T> operator()(T t1, T t2) const {
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
  constexpr std::optional<T> operator()(T t1, T t2) const {
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
  constexpr std::optional<T> operator()(T t1, T t2) const {
    return overflow_multiply(t1, t2);
  }
};
template<>
struct OverflowMultiplies<void> {
  constexpr auto operator()(auto t1, auto t2) const {
    return overflow_multiply(t1, t2);
  }
};
} // namespace thes

#endif // INCLUDE_THESAUROS_MATH_OVERFLOW_HPP
