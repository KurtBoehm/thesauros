// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CHARCONV_STRING_CONVERT_HPP
#define INCLUDE_THESAUROS_CHARCONV_STRING_CONVERT_HPP

#include <charconv>
#include <concepts>
#include <optional>
#include <string_view>
#include <system_error>

namespace thes {
template<std::integral T>
[[nodiscard]] constexpr std::optional<T> string_to_integral(std::string_view sv,
                                                            int base = 10) noexcept {
  T ret{};
  const auto* const begin = sv.data();
  const auto* const end = sv.data() + sv.size();

  const auto res = std::from_chars(begin, end, ret, base);

  if (res.ec == std::errc{} && res.ptr == end) {
    return ret;
  }
  return std::nullopt;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_CHARCONV_STRING_CONVERT_HPP
