// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CHARCONV_PARSE_INTEGER_HPP
#define INCLUDE_THESAUROS_CHARCONV_PARSE_INTEGER_HPP

#include <cstddef>
#include <optional>
#include <string_view>

#include "thesauros/math/overflow.hpp"
#include "thesauros/types/primitives.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes {
/** Governs how `parse_integer` interprets a leading zero and digit separators. */
enum struct IntegerParseMode : thes::u8 {
  /** A leading `0` denotes an octal literal, as in a C++ integer literal; only `'` is skipped. */
  literal,
  /** A leading `0o`/`0O` denotes an octal literal; both `_` and `'` are skipped as separators. */
  extended,
};

/**
 * Parses `src` as a `T`, returning `std::nullopt` if it is empty or not a valid integer literal.
 * Supports `0x`/`0X` (hexadecimal) and `0b`/`0B` (binary) prefixes and, for signed `T`, a leading
 * `-`. See `IntegerParseMode` for how a leading `0` and digit separators are handled.
 */
template<typename T, thes::TypedValueTag<IntegerParseMode> ParseMode =
                       thes::AutoTag<IntegerParseMode::extended>>
[[nodiscard]] constexpr std::optional<T> parse_integer(std::string_view src,
                                                       ParseMode parse_mode = {}) {
  auto parse_impl = [&](std::string_view number, auto op) -> std::optional<T> {
    auto parse_base = [&]<std::size_t Base>(std::string_view sv, IndexTag<Base>) {
      auto parse_char = [](char c) -> std::optional<T> {
        // This slightly convoluted implementation optimizes better.
        const auto digit = [c]() -> std::optional<T> {
          switch (c) {
            case '0': return T{0};
            case '1': return T{1};
            case '2': return T{2};
            case '3': return T{3};
            case '4': return T{4};
            case '5': return T{5};
            case '6': return T{6};
            case '7': return T{7};
            case '8': return T{8};
            case '9': return T{9};
            case 'A': [[fallthrough]];
            case 'a': return T{0xA};
            case 'B': [[fallthrough]];
            case 'b': return T{0xB};
            case 'C': [[fallthrough]];
            case 'c': return T{0xC};
            case 'D': [[fallthrough]];
            case 'd': return T{0xD};
            case 'E': [[fallthrough]];
            case 'e': return T{0xE};
            case 'F': [[fallthrough]];
            case 'f': return T{0xF};
            default: return std::nullopt;
          }
        }();
        if (!digit.has_value() || *digit >= T{Base}) {
          return std::nullopt;
        }
        return digit;
      };

      T v = 0;
      for (char c : sv) {
        if constexpr (parse_mode == IntegerParseMode::extended) {
          if (c == '_') {
            continue;
          }
        }
        if (c == '\'') {
          continue;
        }
        v = op(overflow_multiply(v, T{Base}).valid_value(), parse_char(c).value()).valid_value();
      }
      return v;
    };

    if (number.empty()) {
      return std::nullopt;
    }
    if (number.front() == '0') {
      number.remove_prefix(1);
      if (number.empty()) {
        return T{0};
      }
      char c = number.front();
      if (c == 'x' || c == 'X') {
        return parse_base(number.substr(1), index_tag<16>);
      }
      if (c == 'b' || c == 'B') {
        return parse_base(number.substr(1), index_tag<2>);
      }
      if constexpr (parse_mode == IntegerParseMode::extended) {
        if (c == 'o' || c == 'O') {
          return parse_base(number.substr(1), index_tag<8>);
        }
      } else {
        return parse_base(number, index_tag<8>);
      }
    }
    return parse_base(number, index_tag<10>);
  };

  if (src.empty()) {
    return std::nullopt;
  }
  if (src.front() == '-') {
    return parse_impl(src.substr(1), OverflowMinus<>{});
  }
  return parse_impl(src, OverflowPlus<>{});
}
} // namespace thes

#endif // INCLUDE_THESAUROS_CHARCONV_PARSE_INTEGER_HPP
