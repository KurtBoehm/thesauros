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

#include "thesauros/static-ranges.hpp"
#include "thesauros/string/character-tools.hpp"

namespace thes {
template<std::size_t tSize>
struct StaticString {
  using Value = char;
  using Data = std::array<Value, tSize + 1>;
  static constexpr std::size_t size = tSize;
  static constexpr star::TupleDefsMarker tuple_defs_marker{};

  Data data;

  constexpr StaticString(const char* str)
      : data{star::index_transform<size + 1>([&](auto idx) { return str[idx]; }) | star::to_array} {
  }
  constexpr StaticString(Data&& d) : data{std::move(d)} {}

  static constexpr StaticString filled(char fill) {
    return StaticString{fill};
  }

  template<std::size_t tIdx>
  [[nodiscard]] friend constexpr char get(const StaticString& self) {
    return std::get<tIdx>(self.data);
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

  template<typename... TStrings>
  requires(sizeof...(TStrings) > 0)
  constexpr auto join(const TStrings&... strings) {
    constexpr std::size_t str_num = sizeof...(TStrings);
    constexpr std::size_t full_size = (... + TStrings::size) + (str_num - 1) * size;
    const auto tuple = std::tie(*this, strings...);
    using Tuple = std::tuple<TStrings...>;

    constexpr auto compute_pair = [=](auto i) {
      std::optional<std::pair<std::size_t, std::size_t>> idxs{};
      star::iota<0, str_num> | star::for_each([&](auto j) {
        std::size_t offset = star::index_transform<j>(
                               [](auto k) { return std::tuple_element_t<k, Tuple>::size + size; }) |
                             star::left_reduce(std::plus<>{}, std::size_t{0});

        const std::size_t upper = offset + std::tuple_element_t<j, Tuple>::size;

        if (offset <= i) {
          if (i < upper) {
            assert(!idxs.has_value());
            idxs = std::make_pair(j + 1, i - offset);
          } else if (i < upper + size) {
            assert(!idxs.has_value());
            idxs = std::make_pair(0, i - upper);
          }
        }
      });
      return idxs.value();
    };

    return StaticString<full_size>{star::index_transform<full_size + 1>([&](auto i) {
                                     if constexpr (i < full_size) {
                                       constexpr auto pair = compute_pair(i);
                                       return get<pair.second>(std::get<pair.first>(tuple));
                                     } else {
                                       return '\0';
                                     }
                                   }) |
                                   star::to_array};
  }

  constexpr StaticString<0> join() {
    return {"\0"};
  }

private:
  explicit constexpr StaticString(char fill)
      : data{star::index_transform<size + 1>([&](auto idx) { return (idx < size) ? fill : '\0'; }) |
             star::to_array} {}
};
template<std::size_t tSize>
StaticString(const char (&)[tSize]) -> StaticString<tSize - 1>;

template<std::size_t tSize1, std::size_t tSize2>
inline constexpr bool operator==(const StaticString<tSize1>& s1, const StaticString<tSize2>& s2) {
  if constexpr (tSize1 != tSize2) {
    return false;
  } else {
    return star::index_transform<tSize1>([&](auto i) { return get<i>(s1) == get<i>(s2); }) |
           star::left_reduce(std::logical_and<>{}, true);
  }
}

template<std::size_t tSize1, std::size_t tSize2>
inline constexpr StaticString<tSize1 + tSize2> operator+(const StaticString<tSize1>& s1,
                                                         const StaticString<tSize2>& s2) {
  return StaticString<tSize1 + tSize2>{star::index_transform<tSize1 + tSize2 + 1>([&](auto i) {
                                         if constexpr (i < tSize1) {
                                           return get<i>(s1);
                                         } else if constexpr (i < tSize1 + tSize2) {
                                           return get<i - tSize1>(s2);
                                         } else {
                                           return '\0';
                                         }
                                       }) |
                                       star::to_array};
}

namespace literals {
template<StaticString tString>
inline constexpr auto operator""_sstr() {
  return tString;
}
} // namespace literals

template<StaticString tString>
inline constexpr auto to_snake_case() {
  constexpr std::size_t size = [] {
    std::size_t num = 0;
    for (std::size_t i = 1; i < tString.size; ++i) {
      if (is_uppercase(tString.data[i])) {
        ++num;
      }
    }
    return tString.size + num;
  }();

  std::array<char, size + 1> str{};

  std::size_t j = 0;
  str[j++] = to_lowercase(tString.data[0]);
  for (std::size_t i = 1; i < tString.size; ++i) {
    if (is_uppercase(tString.data[i])) {
      str[j++] = '_';
    }
    str[j++] = to_lowercase(tString.data[i]);
  }
  str[j] = '\0';

  return StaticString<size>(std::move(str));
}
} // namespace thes

#endif // INCLUDE_THESAUROS_STRING_STATIC_STRING_HPP
