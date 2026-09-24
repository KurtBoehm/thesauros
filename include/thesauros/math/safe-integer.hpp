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
  explicit constexpr SafeInt(T value) : value_{value} {}

  [[nodiscard]] T unsafe() const {
    return value_;
  }

  SafeInt operator~() {
    return SafeInt{static_cast<T>(~value_)};
  }

#define THES_SAFE_INT_BINOP_FULL(OP, PREFIX, SAFEINT, TYPE) \
  PREFIX constexpr SafeInt& operator OP## = (SAFEINT amount) { \
    value_ OP## = amount.unsafe(); \
    return *this; \
  } \
  PREFIX constexpr SafeInt& operator OP## = (TYPE amount) { \
    return *this OP## = SafeInt<TYPE>{amount}; \
  } \
  PREFIX friend constexpr SafeInt operator OP(SafeInt self, SAFEINT amount) { \
    return self OP## = amount; \
  } \
  PREFIX friend constexpr SafeInt operator OP(SafeInt self, TYPE amount) { \
    return self OP## = amount; \
  }
#define THES_SAFE_INT_BINOP(OP) THES_SAFE_INT_BINOP_FULL(OP, , SafeInt, T)

  THES_SAFE_INT_BINOP(+)
  THES_SAFE_INT_BINOP(-)
  THES_SAFE_INT_BINOP(<<) // NOLINT(bugprone-signed-bitwise)
  THES_SAFE_INT_BINOP(>>) // NOLINT(bugprone-signed-bitwise)
  THES_SAFE_INT_BINOP(&) // NOLINT(bugprone-signed-bitwise)
  THES_SAFE_INT_BINOP(|) // NOLINT(bugprone-signed-bitwise)

  // NOLINTNEXTLINE(bugprone-signed-bitwise)
  THES_SAFE_INT_BINOP_FULL(<<, template<std::unsigned_integral U>, SafeInt<U>, U)
  // NOLINTNEXTLINE(bugprone-signed-bitwise)
  THES_SAFE_INT_BINOP_FULL(>>, template<std::unsigned_integral U>, SafeInt<U>, U)

#undef THES_SAFE_INT_BINOP
#undef THES_SAFE_INT_BINOP_FULL

  constexpr auto operator<=>(const SafeInt& other) const = default;

private:
  T value_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_MATH_SAFE_INTEGER_HPP
