// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CHARCONV_STRING_ESCAPE_HPP
#define INCLUDE_THESAUROS_CHARCONV_STRING_ESCAPE_HPP

#include <bit>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "thesauros/charconv/unicode.hpp"
#include "thesauros/concepts/type-traits.hpp"
#include "thesauros/math/safe-integer.hpp"
#include "thesauros/string/static-capacity-string.hpp"

namespace thes {
inline auto escape_string(std::string_view in, auto out_it) {
  using enum UnicodeDecoder::State;
  auto extend = [&out_it]<typename... Ts>(Ts... chars) { ((*out_it++ = chars), ...); };

  UnicodeDecoder decoder{};

  const char* end = in.end();
  for (const char* ptr = in.begin(); ptr != end; ++ptr) {
    const auto c = *ptr;

    const auto [codep, state] = decoder.decode(std::bit_cast<std::uint8_t>(c));
    switch (state) {
      case ACCEPTED: {
        switch (codep) {
          case '\b': {
            extend('\\', 'b');
            break;
          }
          case '\t': {
            extend('\\', 't');
            break;
          }
          case '\n': {
            extend('\\', 'n');
            break;
          }
          case '\f': {
            extend('\\', 'f');
            break;
          }
          case '\r': {
            extend('\\', 'r');
            break;
          }
          case '\"': {
            extend('\\', '"');
            break;
          }
          case '\\': {
            extend('\\', '\\');
            break;
          }
          default:
            if (codep <= 0x1F) {
              using SI = SafeInt<char>;
              const SI c1 = SI{'0'} + (SI{c} >> 4);
              const SI c2a = SI{c} & SI{0xF};
              const SI c2b = (c2a < SI{10}) ? (SI{'0'} + c2a) : (SI{'A'} + (c2a - SI{10}));

              extend('\\', 'u', '0', '0', c1.unsafe(), c2b.unsafe());
            } else {
              extend(c);
            }
            break;
        }
        break;
      }
      case REJECTED: {
        throw std::invalid_argument("Found an invalid UTF-8 codepoint!");
      }
      default: {
        extend(c);
        break;
      }
    }
  }

  if (decoder.state() != ACCEPTED) {
    throw std::invalid_argument("The string ends with an incomplete codepoint!");
  }

  return out_it;
}

template<typename TString>
struct StringEscape;
template<std::size_t tSize>
struct StringEscape<char[tSize]> {
  static constexpr auto escape(const char value[tSize]) {
    StaticCapacityString<6 * (tSize - 1)> s{};
    escape_string(value, std::back_inserter(s));
    return s;
  }
};

template<typename T>
struct EscapedPrinter {
  explicit EscapedPrinter(T&& value) : value_(std::forward<T>(value)) {}

  const T& value() const {
    return value_;
  }

private:
  T value_;
};

template<typename T>
static constexpr auto escaped_string(T&& value) {
  if constexpr (CompleteType<StringEscape<T>>) {
    return StringEscape<T>::escape(value);
  } else {
    return EscapedPrinter<T>{std::forward<T>(value)};
  }
}
} // namespace thes

#endif // INCLUDE_THESAUROS_CHARCONV_STRING_ESCAPE_HPP
