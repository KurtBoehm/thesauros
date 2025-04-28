#ifndef INCLUDE_THESAUROS_MATH_DIVMOD_HPP
#define INCLUDE_THESAUROS_MATH_DIVMOD_HPP

#include <concepts>
#include <limits>
#include <utility>

#include "thesauros/types/fixed-size-integer.hpp"
#include "thesauros/types/numeric-info.hpp"
#include "thesauros/types/primitives.hpp"

// The implementation is inspired by https://arxiv.org/abs/1902.01961
// and its implementation in https://github.com/lemire/fastmod.

namespace thes {
namespace detail {
// a: UFixed<0, 16>, b: UFixed<8, 0>
// result: floor(a * b) -> UFixed<8, 0>
constexpr u8 mul_frac_int(u16 a, u8 b) {
  return static_cast<u8>((u32{a} * u32{b}) >> u32{16});
}

// a: UFixed<0, 32>, b: UFixed<16, 0>
// result: floor(a * b) -> UFixed<16, 0>
constexpr u16 mul_frac_int(u32 a, u16 b) {
  return static_cast<u16>((u64{a} * u64{b}) >> u64{32});
}

// a: UFixed<0, 64>, b: UFixed<32, 0>
// result: floor(a * b) -> UFixed<32, 0>
constexpr u32 mul_frac_int(u64 a, u32 b) {
  return static_cast<u32>((u128{a} * u128{b}) >> u128{64});
}

// a: UFixed<0, 128>, b: UFixed<64, 0>
// result: floor(a * b) -> UFixed<64, 0>
constexpr u64 mul_frac_int(u128 a, u64 b) {
  // compute the upper 64 bits of the 192-bit product in three steps:
  // 1. p1 = a[:64] * b >> 64 (the lower bits cannot contribute carry bits)
  // 2. p2 = a[64:] * b (already effectively shifted down by 64 bits)
  // 3: result = (p1 + p2) >> 64
  const auto p1 = (static_cast<u64>(a) * u128{b}) >> u128{64};
  const auto p2 = static_cast<u64>(a >> u128{64}) * u128{b};
  return static_cast<u64>((p1 + p2) >> u128{64});
}
} // namespace detail

template<std::unsigned_integral T>
struct Divisor {
  static constexpr auto byte_num = NumericInfo<T>::byte_num;
  using Value = T;
  using WideValue = FixedUnsignedInt<2 * byte_num>;

  // ceil(4^bit_num / d) for d > 0
  static constexpr WideValue invert(Value d) {
    return static_cast<WideValue>(std::numeric_limits<WideValue>::max() / d + 1);
  }

  explicit constexpr Divisor(T d) : divisor_(d), inverse_(invert(d)) {}

  [[nodiscard]] constexpr friend Value operator%(const Value a, const Divisor& d) {
    const auto low_bits = static_cast<WideValue>(d.inverse_ * a);
    return detail::mul_frac_int(low_bits, d.divisor_);
  }

  [[nodiscard]] constexpr friend Value operator/(const Value a, const Divisor& d) {
    return detail::mul_frac_int(d.inverse_, a) + (d.mask_ & a);
  }

  // checks whether n % d == 0
  [[nodiscard]] constexpr bool is_divisible(Value n) const {
    // This seemingly silly way of writing “<” is necessary for divisor_ == 1,
    // i.e. inverse_ == 0.
    return n * inverse_ <= inverse_ - 1;
  }

  [[nodiscard]] constexpr Value mod_factor() const {
    return mod_factor_;
  }

private:
  Value divisor_;
  WideValue inverse_;
  Value mask_{(inverse_ == 0) ? std::numeric_limits<Value>::max() : Value{0}};
  Value mod_factor_{(inverse_ == 0) ? Value{1} : divisor_};
};

template<typename T>
constexpr std::pair<T, T> divmod(T dividend, T divisor) {
  return {dividend / divisor, dividend % divisor};
}
template<typename T>
constexpr std::pair<T, T> divmod(T dividend, const Divisor<T>& divisor) {
  const auto div = dividend / divisor;
  return {div, dividend - div * divisor.mod_factor()};
}
} // namespace thes

#endif // INCLUDE_THESAUROS_MATH_DIVMOD_HPP
