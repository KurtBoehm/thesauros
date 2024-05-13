#ifndef INCLUDE_THESAUROS_UTILITY_STRING_CONVERT_HPP
#define INCLUDE_THESAUROS_UTILITY_STRING_CONVERT_HPP

#include <charconv>
#include <concepts>
#include <optional>
#include <string_view>
#include <system_error>

namespace thes {
template<std::integral T>
inline std::optional<T> string_to_integral(std::string_view sv) {
  T ret{};
  const auto res = std::from_chars(sv.begin(), sv.end(), ret);
  if (res.ec == std::errc{} && res.ptr == sv.end()) {
    return ret;
  }
  return std::nullopt;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_STRING_CONVERT_HPP
