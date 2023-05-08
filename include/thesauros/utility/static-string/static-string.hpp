#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_STRING_STATIC_STRING_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_STRING_STATIC_STRING_HPP

#include <array>
#include <cstddef>
#include <string_view>

#include "thesauros/utility/static-string/character-tools.hpp"
#include "thesauros/utility/static-value.hpp"

namespace thes {
template<std::size_t tSize>
struct StaticString {
  static constexpr std::size_t size = tSize;
  using Data = std::array<char, size + 1>;

  Data data;

  constexpr StaticString(const char* str) : data{mapped_data([&](auto idx) { return str[idx]; })} {}

  static constexpr StaticString filled(char fill) {
    return StaticString{fill};
  }

  template<std::size_t tIdx>
  [[nodiscard]] constexpr char get() const {
    return std::get<tIdx>(data);
  }

  [[nodiscard]] constexpr std::string_view view() const {
    return {data.data(), size};
  }

  [[nodiscard]] constexpr auto to_lowercase() const {
    Data str{};
    for (std::size_t i = 0; i < size; ++i) {
      str[i] = thes::to_lowercase(data[i]);
    }
    str[size] = '\0';
    return StaticString<size>(str.data());
  }

private:
  constexpr explicit StaticString(char fill)
      : data{mapped_data([&](auto idx) { return (idx < size) ? fill : '\0'; })} {}

  static constexpr auto mapped_data(auto fun) {
    return [&]<std::size_t... tIdxs>(std::index_sequence<tIdxs...>) {
      return Data{fun(static_auto<tIdxs>)...};
    }(std::make_index_sequence<size + 1>{});
  }
};
template<std::size_t tSize>
StaticString(const char (&)[tSize]) -> StaticString<tSize - 1>;

namespace literals {
template<StaticString tString>
inline constexpr auto operator""_sstr() {
  return tString;
}
} // namespace literals

template<StaticString tString>
inline constexpr std::size_t snake_case_size() {
  std::size_t num = 0;
  for (std::size_t i = 1; i < tString.size; ++i) {
    if (is_uppercase(tString.data[i])) {
      ++num;
    }
  }
  return tString.size + num;
}
template<StaticString tString>
inline constexpr auto to_snake_case() {
  std::array<char, snake_case_size<tString>() + 1> str{};

  std::size_t j = 0;
  str[j++] = to_lowercase(tString.data[0]);
  for (std::size_t i = 1; i < tString.size; ++i) {
    if (is_uppercase(tString.data[i])) {
      str[j++] = '_';
    }
    str[j++] = to_lowercase(tString.data[i]);
  }
  str[j] = '\0';

  return StaticString<snake_case_size<tString>()>(str.data());
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_STRING_STATIC_STRING_HPP
