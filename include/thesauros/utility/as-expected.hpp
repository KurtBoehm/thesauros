// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_AS_EXPECTED_HPP
#define INCLUDE_THESAUROS_UTILITY_AS_EXPECTED_HPP

#include <concepts>
#include <expected>

namespace thes {
template<std::integral T>
constexpr std::expected<void, T> as_expected(T ret) {
  using Out = std::expected<void, T>;

  Out out{};
  if (ret != 0) {
    out = std::unexpected(ret);
  }

  return out;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_AS_EXPECTED_HPP
