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

  T unsafe() const {
    return value_;
  }

#define THES_SAFE_INT_BINOP(OP) \
  constexpr SafeInt& operator OP##=(SafeInt amount) { \
    value_ OP## = amount.unsafe(); \
    return *this; \
  } \
  constexpr SafeInt& operator OP##=(T amount) { \
    return *this OP## = SafeInt<T>{amount}; \
  } \
  friend constexpr SafeInt operator OP(SafeInt self, SafeInt amount) { \
    return self OP## = amount; \
  } \
  friend constexpr SafeInt operator OP(SafeInt self, T amount) { \
    return self OP## = amount; \
  }

  THES_SAFE_INT_BINOP(+)
  THES_SAFE_INT_BINOP(-)
  THES_SAFE_INT_BINOP(<<)
  THES_SAFE_INT_BINOP(>>)
  THES_SAFE_INT_BINOP(&)
  THES_SAFE_INT_BINOP(|)

#undef THES_SAFE_INT_BINOP

  constexpr auto operator<=>(const SafeInt& other) const = default;

private:
  T value_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_MATH_SAFE_INTEGER_HPP
