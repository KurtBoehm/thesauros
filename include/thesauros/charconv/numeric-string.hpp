// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CHARCONV_NUMERIC_STRING_HPP
#define INCLUDE_THESAUROS_CHARCONV_NUMERIC_STRING_HPP

#include <algorithm>
#include <charconv>
#include <concepts>
#include <cstddef>
#include <expected>
#include <limits>
#include <system_error>

#include "thesauros/concepts/type-traits.hpp"
#include "thesauros/math/arithmetic.hpp"
#include "thesauros/math/integer-cast.hpp"
#include "thesauros/string/static-capacity-string.hpp"

namespace thes {
/** The maximum number of characters `numeric_string` may need to represent a `Numeric` value. */
template<Numeric T>
inline constexpr unsigned max_char_num = [] {
  using Limits = std::numeric_limits<T>;
  if constexpr (std::floating_point<T>) {
    // 4: sign, decimal point, and “e+” or “e-”.
    // max_digits10: the significand.
    // max(2, max_exponent10): the exponent, which has at least two digits.
    return 4U + unsigned{Limits::max_digits10} +
           std::max(2U, abs_log_ceil(10U, unsigned{Limits::max_exponent10}));
  } else if constexpr (std::unsigned_integral<T>) {
    // Unsigned values need no sign character.
    return abs_log_ceil(T{10}, Limits::max());
  } else {
    // 1: sign.
    return 1 + abs_log_ceil(T{10}, Limits::lowest());
  }
}();

/**
 * Converts `value` to its shortest round-trippable textual representation.
 * Only usable in a constant expression for integral `T`: `std::to_chars` for floating-point types
 * is not `constexpr` as of C++23.
 */
template<Numeric T>
[[nodiscard]] constexpr std::expected<StaticCapacityString<max_char_num<T>>, std::errc>
numeric_string(const T& value) {
  StaticCapacityString<max_char_num<T>> out{};
  auto res = std::to_chars(out.data(), out.data() + max_char_num<T>, value);
  if (res.ec == std::errc{}) {
    out.set_size(safe_cast<std::size_t>(res.ptr - out.data()).valid_value());
    return out;
  }
  return std::unexpected{res.ec};
}
} // namespace thes

#endif // INCLUDE_THESAUROS_CHARCONV_NUMERIC_STRING_HPP
