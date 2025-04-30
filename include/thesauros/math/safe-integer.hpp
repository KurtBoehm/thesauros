// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_MATH_SAFE_INTEGER_HPP
#define INCLUDE_THESAUROS_MATH_SAFE_INTEGER_HPP

#include <concepts>

namespace thes {
template<std::integral T>
struct SafeInt {
  explicit constexpr SafeInt(T value) : value_(value) {}

  T raw() const {
    return value_;
  }

  constexpr SafeInt& operator+=(SafeInt other) {
    value_ += other.value_;
    return *this;
  }
  friend constexpr SafeInt operator+(SafeInt a, SafeInt b) {
    return a += b;
  }

  constexpr SafeInt& operator-=(SafeInt other) {
    value_ -= other.value_;
    return *this;
  }
  friend constexpr SafeInt operator-(SafeInt a, SafeInt b) {
    return a -= b;
  }

  template<std::unsigned_integral TOther>
  constexpr SafeInt& operator>>=(SafeInt<TOther> amount) {
    value_ >>= amount.raw();
    return *this;
  }
  template<typename TOther>
  constexpr SafeInt& operator>>=(TOther amount) {
    return *this >>= SafeInt<TOther>{amount};
  }
  friend constexpr SafeInt operator>>(SafeInt self, auto amount) {
    return self >>= amount;
  }

  constexpr SafeInt& operator&=(SafeInt other) {
    value_ &= other.raw();
    return *this;
  }
  constexpr SafeInt& operator&=(T other) {
    return *this &= SafeInt{other};
  }
  friend constexpr SafeInt operator&(SafeInt a, auto b) {
    return a &= b;
  }

  constexpr auto operator<=>(const SafeInt& other) const = default;

private:
  T value_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_MATH_SAFE_INTEGER_HPP
