#ifndef INCLUDE_THESAUROS_CHARCONV_NUMERIC_STRING_HPP
#define INCLUDE_THESAUROS_CHARCONV_NUMERIC_STRING_HPP

#include <algorithm>
#include <charconv>
#include <concepts>
#include <cstddef>
#include <limits>
#include <system_error>

#include "tl/expected.hpp"

#include "thesauros/math/arithmetic.hpp"
#include "thesauros/math/integer-cast.hpp"
#include "thesauros/string/static-capacity-string.hpp"

namespace thes {
template<typename T>
struct MaxCharNumTrait;
template<std::floating_point T>
struct MaxCharNumTrait<T> {
  using Type = T;
  using Limits = std::numeric_limits<Type>;

  // 4: sign, decimal point, and “e+” or “e-”
  // max_digits10: significand
  // max(2, max_exponent10): Exponent, which has at least two digits
  static constexpr auto char_num =
    4U + unsigned{Limits::max_digits10} +
    std::max(2U, abs_log_ceil(10U, unsigned{Limits::max_exponent10}));
};
template<std::unsigned_integral T>
struct MaxCharNumTrait<T> {
  using Type = T;
  using Limits = std::numeric_limits<Type>;

  // 1: sign
  static constexpr auto char_num = abs_log_ceil(Type{10}, Limits::max());
};
template<std::signed_integral T>
struct MaxCharNumTrait<T> {
  using Type = T;
  using Limits = std::numeric_limits<Type>;

  // 1: sign
  static constexpr auto char_num = 1 + abs_log_ceil(Type{10}, Limits::lowest());
};
template<typename T>
inline constexpr unsigned max_char_num = MaxCharNumTrait<T>::char_num;

template<typename T>
constexpr tl::expected<StaticCapacityString<max_char_num<T>>, std::errc>
numeric_string(const T& value) {
  StaticCapacityString<max_char_num<T>> out{};
  auto res = std::to_chars(out.data(), out.data() + max_char_num<T>, value);
  if (res.ec == std::errc{}) {
    out.size() = safe_cast<std::size_t>(res.ptr - out.data()).valid_value();
    return out;
  }
  return tl::unexpected(res.ec);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_CHARCONV_NUMERIC_STRING_HPP
