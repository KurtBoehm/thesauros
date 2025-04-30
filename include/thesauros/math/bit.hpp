// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_MATH_BIT_HPP
#define INCLUDE_THESAUROS_MATH_BIT_HPP

#include <bit>
#include <concepts>

namespace thes {
template<std::unsigned_integral T>
inline constexpr unsigned bit_width(T x) noexcept {
  return static_cast<unsigned>(std::bit_width(x));
}
template<std::unsigned_integral T>
inline constexpr unsigned countr_zero(T x) noexcept {
  return static_cast<unsigned>(std::countr_zero(x));
}
template<std::unsigned_integral T>
inline constexpr unsigned countr_one(T x) noexcept {
  return static_cast<unsigned>(std::countr_one(x));
}
} // namespace thes

#endif // INCLUDE_THESAUROS_MATH_BIT_HPP
