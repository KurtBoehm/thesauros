// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_UNWRAP_HPP
#define INCLUDE_THESAUROS_UTILITY_UNWRAP_HPP

#include <functional>
#include <utility>

namespace thes {
template<typename T>
inline constexpr T&& unwrap(T&& value) {
  return std::forward<T>(value);
}
template<typename T>
inline constexpr T& unwrap(std::reference_wrapper<T> value) {
  return value;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_UNWRAP_HPP
