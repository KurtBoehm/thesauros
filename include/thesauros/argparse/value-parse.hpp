// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_ARGPARSE_VALUE_PARSE_HPP
#define INCLUDE_THESAUROS_ARGPARSE_VALUE_PARSE_HPP

#include <charconv>
#include <concepts>
#include <cstddef>
#include <limits>
#include <optional>
#include <string_view>
#include <system_error>
#include <type_traits>

#include "thesauros/string/character-tools.hpp"
#include "thesauros/types/type-tag.hpp"

namespace thes::argparse {
namespace detail {
/** The value of `c` read as a base-16 digit, or `std::nullopt` if it is no digit at all. */
constexpr std::optional<unsigned> digit_value(char c) {
  if (c >= '0' && c <= '9') {
    return static_cast<unsigned>(c - '0');
  }
  const char lower = to_lowercase(c);
  if (lower >= 'a' && lower <= 'f') {
    return static_cast<unsigned>(lower - 'a') + 10U;
  }
  return std::nullopt;
}

/** Whether `text` equals `expected`, ignoring the case of ASCII letters. */
constexpr bool equals_ignoring_case(std::string_view text, std::string_view expected) {
  if (text.size() != expected.size()) {
    return false;
  }
  for (std::size_t i = 0; i < text.size(); ++i) {
    if (to_lowercase(text[i]) != expected[i]) {
      return false;
    }
  }
  return true;
}
} // namespace detail

/**
 * Parses `text` as an integer of type `T`, returning `std::nullopt` for malformed input and for
 * values `T` cannot represent. Accepts a leading `+` or `-`, the base prefixes `0x`/`0X`, `0o`/`0O`
 * and `0b`/`0B`, and `_` and `'` as separators between digits.
 *
 * Unlike `thes::parse_integer`, this rejects out-of-range and malformed input by returning
 * `std::nullopt` rather than by throwing, which is what a parser fed unvalidated input needs.
 */
template<std::integral T>
requires(!std::same_as<T, bool>)
[[nodiscard]] constexpr std::optional<T> parse_integer_value(std::string_view text) {
  using Unsigned = std::make_unsigned_t<T>;

  bool negative = false;
  if (!text.empty() && (text.front() == '+' || text.front() == '-')) {
    negative = text.front() == '-';
    text.remove_prefix(1);
  }
  if constexpr (std::unsigned_integral<T>) {
    if (negative) {
      return std::nullopt;
    }
  }

  unsigned base = 10;
  if (text.size() > 2 && text.front() == '0') {
    switch (to_lowercase(text[1])) {
      case 'x': base = 16; break;
      case 'o': base = 8; break;
      case 'b': base = 2; break;
      default: break;
    }
    if (base != 10) {
      text.remove_prefix(2);
    }
  }

  // The magnitude is accumulated in the unsigned counterpart of `T` so that the overflow check
  // stays well defined and the most negative value of a signed `T` remains reachable.
  constexpr auto highest = static_cast<Unsigned>(std::numeric_limits<T>::max());
  const Unsigned limit = negative ? static_cast<Unsigned>(highest + Unsigned{1}) : highest;

  Unsigned value{0};
  bool after_digit = false;
  for (const char c : text) {
    if (c == '_' || c == '\'') {
      if (!after_digit) {
        return std::nullopt;
      }
      after_digit = false;
      continue;
    }
    const std::optional<unsigned> digit = detail::digit_value(c);
    if (!digit.has_value() || *digit >= base) {
      return std::nullopt;
    }
    const auto added = static_cast<Unsigned>(*digit);
    if (added > limit || value > static_cast<Unsigned>((limit - added) / base)) {
      return std::nullopt;
    }
    value = static_cast<Unsigned>(value * base + added);
    after_digit = true;
  }
  if (!after_digit) {
    return std::nullopt;
  }

  if constexpr (std::signed_integral<T>) {
    // The wrapping conversion is deliberate: it turns the accumulated magnitude into the intended
    // negative value, including the most negative one, which has no representable negation.
    return negative ? static_cast<T>(static_cast<Unsigned>(Unsigned{0} - value))
                    : static_cast<T>(value);
  } else {
    return static_cast<T>(value);
  }
}

/**
 * Parses `text` as a boolean: `true`, `yes`, `on` and `1` yield `true`, `false`, `no`, `off` and
 * `0` yield `false`, all ignoring case.
 */
[[nodiscard]] constexpr std::optional<bool> parse_boolean_value(std::string_view text) {
  for (const std::string_view name : {"true", "yes", "on", "1"}) {
    if (detail::equals_ignoring_case(text, name)) {
      return true;
    }
  }
  for (const std::string_view name : {"false", "no", "off", "0"}) {
    if (detail::equals_ignoring_case(text, name)) {
      return false;
    }
  }
  return std::nullopt;
}

/**
 * Parses all of `text` as a floating-point value in the syntax of `std::from_chars`, which is that
 * of `std::strtod` without a leading `+` and without the locale.
 */
template<std::floating_point T>
[[nodiscard]] inline std::optional<T> parse_floating_value(std::string_view text) {
  T value{};
  const char* const end = text.data() + text.size();
  const std::from_chars_result res = std::from_chars(text.data(), end, value);
  if (res.ec == std::errc{} && res.ptr == end) {
    return value;
  }
  return std::nullopt;
}

namespace detail {
/** Whether `parse_argument_value` is defined for `T` and findable by argument-dependent lookup. */
template<typename T>
concept CustomValue = requires(std::string_view text) {
  { parse_argument_value(type_tag<T>, text) } -> std::same_as<std::optional<T>>;
};
} // namespace detail

/**
 * The types usable as the value type of an argument: `std::string_view`, `bool`, the integral and
 * floating-point types, and any type for which `parse_argument_value(TypeTag<T>, std::string_view)`
 * returning `std::optional<T>` is findable by argument-dependent lookup.
 */
template<typename T>
concept ArgumentValue = detail::CustomValue<T> || std::same_as<T, std::string_view> ||
                        std::integral<T> || std::floating_point<T>;

/**
 * Converts `text` to a `T`, returning `std::nullopt` if it is no valid representation of one.
 * A custom `parse_argument_value` takes precedence over the built-in conversions.
 */
template<ArgumentValue T>
[[nodiscard]] constexpr std::optional<T> parse_value(std::string_view text) {
  if constexpr (detail::CustomValue<T>) {
    return parse_argument_value(type_tag<T>, text);
  } else if constexpr (std::same_as<T, std::string_view>) {
    return text;
  } else if constexpr (std::same_as<T, bool>) {
    return parse_boolean_value(text);
  } else if constexpr (std::integral<T>) {
    return parse_integer_value<T>(text);
  } else {
    return parse_floating_value<T>(text);
  }
}
} // namespace thes::argparse

#endif // INCLUDE_THESAUROS_ARGPARSE_VALUE_PARSE_HPP
