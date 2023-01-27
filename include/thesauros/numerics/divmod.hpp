#ifndef INCLUDE_THESAUROS_NUMERICS_DIVMOD_HPP
#define INCLUDE_THESAUROS_NUMERICS_DIVMOD_HPP

#include <utility>

#include "thesauros/utility/fixed-size-integer.hpp"
#include "thesauros/utility/numeric-info.hpp"
#include "thesauros/utility/primitives.hpp"

// The implementation is inspired by https://arxiv.org/abs/1902.01961
// and its implementation on https://github.com/lemire/fastmod.

namespace thes {
namespace divmod_impl {
// a: UFixed<0, 16>, b: UFixed<8, 0>
// result: floor(a * b) -> UFixed<8, 0>
inline constexpr U8 mul_frac_int(U16 a, U8 b) {
  return static_cast<U8>((U32{a} * U32{b}) >> U32{16});
}

// a: UFixed<0, 32>, b: UFixed<16, 0>
// result: floor(a * b) -> UFixed<16, 0>
inline constexpr U16 mul_frac_int(U32 a, U16 b) {
  return static_cast<U16>((U64{a} * U64{b}) >> U64{32});
}

// a: UFixed<0, 64>, b: UFixed<32, 0>
// result: floor(a * b) -> UFixed<32, 0>
inline constexpr U32 mul_frac_int(U64 a, U32 b) {
  return static_cast<U32>((U128{a} * U128{b}) >> U128{64});
}

// a: UFixed<0, 128>, b: UFixed<64, 0>
// result: floor(a * b) -> UFixed<64, 0>
inline constexpr U64 mul_frac_int(U128 a, U64 b) {
  // compute the upper 64 bits of the 192-bit product in three steps:
  // 1. p1 = a[:64] * b >> 64 (the lower bits cannot contribute carry bits)
  // 2. p2 = a[64:] * b (already effectively shifted down by 64 bits)
  // 3: result = (p1 + p2) >> 64
  const auto p1 = ((a & std::numeric_limits<U64>::max()) * U128{b}) >> U128{64};
  const auto p2 = (a >> U128{64}) * b;
  const auto result = (p1 + p2) >> U128{64};
  return static_cast<U64>(result);
}
} // namespace divmod_impl

template<std::unsigned_integral T>
struct Divisor {
  using Value = T;
  using WideValue = FixedUnsignedInt<2 * NumericInfo<T>::byte_num>;

  // ceil(4^bit_num / d) for d > 0
  static constexpr WideValue invert(Value d) {
    return std::numeric_limits<WideValue>::max() / d + 1;
  }

  constexpr explicit Divisor(T d) : divisor_(d), inverse_(invert(d)) {}

  [[nodiscard]] inline constexpr friend Value operator%(const Value a, const Divisor& d) {
    const auto low_bits = d.inverse_ * a;
    return divmod_impl::mul_frac_int(low_bits, d.divisor_);
  }

  [[nodiscard]] inline constexpr friend Value operator/(const Value a, const Divisor& d) {
    return divmod_impl::mul_frac_int(d.inverse_, a) + (d.mask_ & a);
  }

  // checks whether n % d == 0
  [[nodiscard]] inline constexpr bool is_divisible(Value n) const {
    // This seemingly silly way of writing “<” is necessary for divisor_ == 1,
    // i.e. inverse_ == 0.
    return n * inverse_ <= inverse_ - 1;
  }

  [[nodiscard]] inline constexpr Value get() const {
    return (inverse_ == 0) ? 1 : divisor_;
  }

private:
  Value divisor_;
  WideValue inverse_;
  Value mask_{(inverse_ == 0) ? std::numeric_limits<Value>::max() : 0};
};

template<typename T>
inline constexpr std::pair<T, T> divmod(T dividend, T divisor) {
  return {dividend / divisor, dividend % divisor};
}
template<typename T>
inline constexpr std::pair<T, T> divmod(T dividend, const Divisor<T>& divisor) {
  const auto div = dividend / divisor;
  return {div, dividend - div * divisor.get()};
}
} // namespace thes

#endif // INCLUDE_THESAUROS_NUMERICS_DIVMOD_HPP
