#ifndef INCLUDE_THESAUROS_CHARCONV_PARSE_INTEGER_HPP
#define INCLUDE_THESAUROS_CHARCONV_PARSE_INTEGER_HPP

#include <cstddef>
#include <limits>
#include <optional>
#include <string_view>

#include "thesauros/math/overflow.hpp"
#include "thesauros/types/primitives.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes {
enum struct IntegerParseMode : thes::u8 { literal, extended };

template<typename T, thes::TypedValueTag<IntegerParseMode> TParseMode =
                       thes::AutoTag<IntegerParseMode::extended>>
constexpr std::optional<T> parse_integer(std::string_view src, TParseMode parse_mode = {}) {
  auto parse_impl = [&](std::string_view number, auto op) -> std::optional<T> {
    auto parse_base = [&]<std::size_t tBase>(std::string_view sv, IndexTag<tBase>) {
      auto parse_char = [&](char c) -> std::optional<T> {
        // This slightly convoluted implementation optimizes better
        auto char_map = [c]() -> T {
          switch (c) {
          case '0': return 0;
          case '1': return 1;
          case '2': return 2;
          case '3': return 3;
          case '4': return 4;
          case '5': return 5;
          case '6': return 6;
          case '7': return 7;
          case '8': return 8;
          case '9': return 9;
          case 'A':
          case 'a': return 0xa;
          case 'B':
          case 'b': return 0xb;
          case 'C':
          case 'c': return 0xc;
          case 'D':
          case 'd': return 0xd;
          case 'E':
          case 'e': return 0xe;
          case 'F':
          case 'f': return 0xf;
          default: return std::numeric_limits<T>::max();
          }
        };

        const auto v = char_map();
        if (v < T{tBase}) {
          return v;
        }
        return std::nullopt;
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
        v = op(overflow_multiply(v, T{tBase}).valid_value(), parse_char(c).value()).valid_value();
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
