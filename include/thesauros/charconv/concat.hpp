// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CHARCONV_CONCAT_HPP
#define INCLUDE_THESAUROS_CHARCONV_CONCAT_HPP

#include <cassert>
#include <concepts>
#include <string>
#include <string_view>

#include "thesauros/charconv/numeric-string.hpp"
#include "thesauros/concepts/type-traits.hpp"

namespace thes {
namespace detail {
/** Appends `value` to `out` verbatim. */
inline void concat_append(std::string& out, std::string_view value) {
  out.append(value);
}
/** Appends the single character `value` to `out`. */
inline void concat_append(std::string& out, char value) {
  out.push_back(value);
}
/**
 * Appends `value` to `out` as “true” or “false”.
 * This is constrained rather than taking `bool` directly because a pointer converts to `bool` by a
 * standard conversion, which would beat the user-defined conversion to `std::string_view` and make
 * every string literal print as “true”.
 */
template<std::same_as<bool> T>
inline void concat_append(std::string& out, T value) {
  out.append(value ? "true" : "false");
}
/** Appends the shortest round-trippable representation of the number `value` to `out`. */
template<Numeric T>
requires (!std::same_as<T, bool>)
inline void concat_append(std::string& out, T value) {
  const auto str = numeric_string(value);
  assert(str.has_value());
  out.append(std::string_view{*str});
}
} // namespace detail

/**
 * Concatenates the textual representations of `args...` into a `std::string`.
 * Strings are copied verbatim, `char` is appended as a single character, `bool` becomes “true” or
 * “false”, and other numbers are converted as by `numeric_string`.
 *
 * This is deliberately not a formatting facility: it exists so that the parts of Thesauros below
 * `format` can build messages, above all for exceptions, without depending on `{fmt}`.
 */
template<typename... TArgs>
[[nodiscard]] inline std::string cat(const TArgs&... args) {
  std::string out{};
  (detail::concat_append(out, args), ...);
  return out;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_CHARCONV_CONCAT_HPP
