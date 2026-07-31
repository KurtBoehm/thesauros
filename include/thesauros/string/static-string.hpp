// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STRING_STATIC_STRING_HPP
#define INCLUDE_THESAUROS_STRING_STATIC_STRING_HPP

#include <array>
#include <cassert>
#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>
#include <tuple>
#include <utility>

#include "thesauros/static-ranges/definitions/static-apply.hpp"
#include "thesauros/static-ranges/definitions/tuple-defs.hpp"
#include "thesauros/static-ranges/piping.hpp" // IWYU pragma: keep
#include "thesauros/static-ranges/sinks/reduce.hpp"
#include "thesauros/static-ranges/sinks/to-array.hpp"
#include "thesauros/static-ranges/views/transform.hpp"
#include "thesauros/string/character-tools.hpp"

namespace thes {
template<std::size_t Size>
struct StaticString {
  using Value = char;
  using Data = std::array<Value, Size + 1>;
  static constexpr std::size_t size = Size;
  static constexpr star::TupleDefsMarker tuple_defs_marker{};

  Data data;

  constexpr StaticString(const char* str)
      : data{star::static_apply<size + 1>(
          [str]<std::size_t... I>() { return std::array{str[I]...}; })} {}
  constexpr StaticString(Data&& d) : data{std::move(d)} {}

  static constexpr StaticString filled(char fill) {
    return StaticString{fill};
  }

  template<std::size_t I>
  [[nodiscard]] friend constexpr char get(const StaticString& self) {
    return std::get<I>(self.data);
  }

  [[nodiscard]] constexpr std::string_view view() const {
    return {data.data(), size};
  }

  friend std::string_view format_as(const StaticString& self) {
    return self.view();
  }

  [[nodiscard]] constexpr auto to_lowercase() const {
    Data str{};
    for (std::size_t i = 0; i < size; ++i) {
      str[i] = thes::to_lowercase(data[i]);
    }
    str[size] = '\0';
    return StaticString<size>(std::move(str));
  }

  template<typename... Strings>
  requires(sizeof...(Strings) > 0)
  constexpr auto join(const Strings&... strings) {
    constexpr std::size_t str_num = sizeof...(Strings);
    constexpr std::size_t full_size = (... + Strings::size) + (str_num - 1) * size;
    const auto tuple = std::tie(strings...);
    using Tuple = std::tuple<Strings...>;

    // number of characters before string i and an array of these
    constexpr auto prefix_size = [=](auto i) {
      return star::static_apply<i>([]<std::size_t... I>() {
        return (std::size_t{0} + ... +
                (std::tuple_element_t<I, Tuple>::size + ((I + 1 < str_num) ? size : 0)));
      });
    };
    constexpr std::array<std::size_t, str_num + 1> prefix_sizes = star::static_apply<str_num + 1>(
      [=]<std::size_t... I>() { return std::array{prefix_size(index_tag<I>)...}; });
    // find the string (including the following copy of *this) which output index i falls into
    constexpr auto find_str = [=](auto i) {
      for (std::size_t j = 0; j < str_num; ++j) {
        if (prefix_sizes[j] <= i && i < prefix_sizes[j + 1]) {
          return std::optional{j};
        }
      }
      return std::optional<std::size_t>{};
    };
    // get the character at index i
    const auto get_char = [&, this](auto i) {
      constexpr auto off = find_str(i).value();
      constexpr auto j = i - prefix_sizes[off];
      constexpr auto offsize = std::tuple_element_t<off, Tuple>::size;
      if constexpr (j < offsize) {
        return get<j>(std::get<off>(tuple));
      } else {
        return get<j - offsize>(*this);
      }
    };

    return StaticString<full_size>{star::static_apply<full_size>(
      [=]<std::size_t... I>() { return std::array{get_char(index_tag<I>)..., '\0'}; })};
  }

  constexpr StaticString<0> join() {
    return {"\0"};
  }

private:
  explicit constexpr StaticString(char fill)
      : data{star::index_transform<size + 1>([&](auto idx) { return (idx < size) ? fill : '\0'; }) |
             star::to_array} {}
};
template<std::size_t Size>
StaticString(const char (&)[Size]) -> StaticString<Size - 1>;

template<std::size_t Size1, std::size_t Size2>
constexpr bool operator==(const StaticString<Size1>& s1, const StaticString<Size2>& s2) {
  if constexpr (Size1 != Size2) {
    return false;
  } else {
    return star::index_transform<Size1>([&](auto i) { return get<i>(s1) == get<i>(s2); }) |
           star::left_reduce(std::logical_and<>{}, true);
  }
}

template<std::size_t Size1, std::size_t Size2>
constexpr StaticString<Size1 + Size2> operator+(const StaticString<Size1>& s1,
                                                const StaticString<Size2>& s2) {
  return StaticString<Size1 + Size2>{star::index_transform<Size1 + Size2 + 1>([&](auto i) {
                                       if constexpr (i < Size1) {
                                         return get<i>(s1);
                                       } else if constexpr (i < Size1 + Size2) {
                                         return get<i - Size1>(s2);
                                       } else {
                                         return '\0';
                                       }
                                     }) |
                                     star::to_array};
}

inline namespace literals {
inline namespace static_string_literals {
template<StaticString String>
constexpr auto operator""_sstr() {
  return String;
}
} // namespace static_string_literals
} // namespace literals

template<StaticString String>
constexpr auto to_snake_case() {
  constexpr std::size_t size = [] {
    std::size_t num = 0;
    for (std::size_t i = 1; i < String.size; ++i) {
      if (is_uppercase(String.data[i])) {
        ++num;
      }
    }
    return String.size + num;
  }();

  std::array<char, size + 1> str{};

  std::size_t j = 0;
  str[j++] = to_lowercase(String.data[0]);
  for (std::size_t i = 1; i < String.size; ++i) {
    if (is_uppercase(String.data[i])) {
      str[j++] = '_';
    }
    str[j++] = to_lowercase(String.data[i]);
  }
  str[j] = '\0';

  return StaticString<size>(std::move(str));
}
} // namespace thes

#endif // INCLUDE_THESAUROS_STRING_STATIC_STRING_HPP
