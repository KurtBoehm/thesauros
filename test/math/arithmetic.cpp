// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <bit>
#include <cmath>
#include <limits>

#include "thesauros/math/arithmetic.hpp"
#include "thesauros/math/bit.hpp"
#include "thesauros/math/compile-time.hpp"
#include "thesauros/math/float-fraction.hpp"
#include "thesauros/math/safe-integer.hpp"
#include "thesauros/test/test.hpp"
#include "thesauros/types/primitives.hpp"

namespace {
//==================================================================================================
// Compile-time evaluation
//==================================================================================================

static_assert(thes::div_ceil(7, 3) == 3);
static_assert(thes::pow(3U, 4U) == 81);
static_assert(thes::pow<4>(3) == 81);
static_assert(thes::log2_floor(5U) == 2);
static_assert(thes::log2_ceil(5U) == 3);
static_assert(thes::bit_mask<thes::u8>(5) == 7);
static_assert(thes::combine_bits<thes::u8>(true, false, true) == 5);

// `abs_log_ceil` is `consteval`, so a constant expression is the only way to call it: it counts the
// digits needed to write `num` in base `base`, ignoring the sign.
static_assert(thes::abs_log_ceil(10, 0) == 1);
static_assert(thes::abs_log_ceil(10, 9) == 1);
static_assert(thes::abs_log_ceil(10, 10) == 2);
static_assert(thes::abs_log_ceil(10, 999) == 3);
static_assert(thes::abs_log_ceil(10, -999) == 3);
static_assert(thes::abs_log_ceil(2, 8) == 4);

//==================================================================================================
// Division and exponentiation
//==================================================================================================

/** Checks that `div_ceil` rounds up exactly when the division leaves a remainder. */
THES_TEST_CASE("div_ceil rounds up on a remainder", "[math][arithmetic]") {
  THES_CHECK(thes::div_ceil(0U, 3U) == 0);
  THES_CHECK(thes::div_ceil(1U, 3U) == 1);
  THES_CHECK(thes::div_ceil(3U, 3U) == 1);
  THES_CHECK(thes::div_ceil(4U, 3U) == 2);
  THES_CHECK(thes::div_ceil(6U, 3U) == 2);
  THES_CHECK(thes::div_ceil(7U, 1U) == 7);
}

/** Checks the run-time exponent overload of `pow` against repeated multiplication. */
THES_TEST_CASE("pow with a run-time exponent", "[math][arithmetic]") {
  THES_CHECK(thes::pow(2U, 0U) == 1);
  THES_CHECK(thes::pow(2U, 1U) == 2);
  THES_CHECK(thes::pow(0U, 3U) == 0);
  THES_CHECK(thes::pow(1U, 63U) == 1);

  for (unsigned exponent = 0; exponent < 20; ++exponent) {
    thes::u64 expected = 1;
    for (unsigned i = 0; i < exponent; ++i) {
      expected *= 3;
    }
    THES_CHECK(thes::pow(thes::u64{3}, exponent) == expected);
  }
}

/** Checks the compile-time exponent overload of `pow`, including the `0` and `1` base cases. */
THES_TEST_CASE("pow with a compile-time exponent", "[math][arithmetic]") {
  THES_CHECK(thes::pow<0>(7) == 1);
  THES_CHECK(thes::pow<1>(7) == 7);
  THES_CHECK(thes::pow<2>(7) == 49);
  THES_CHECK(thes::pow<3>(7) == 343);
  THES_CHECK(thes::pow<5>(2) == 32);
  THES_CHECK(thes::pow<3>(2.0) == 8.0);
}

//==================================================================================================
// Logarithms and bit masks
//==================================================================================================

/** Checks `log2_floor`/`log2_ceil` against `std::bit_width` on every power of two and its
 * neighbours. */
THES_TEST_CASE("log2_floor and log2_ceil bracket the exact logarithm", "[math][arithmetic]") {
  THES_CHECK(thes::log2_floor(1U) == 0);
  THES_CHECK(thes::log2_ceil(1U) == 0);
  THES_CHECK(thes::log2_floor(2U) == 1);
  THES_CHECK(thes::log2_ceil(2U) == 1);
  THES_CHECK(thes::log2_ceil(3U) == 2);

  // From `shift == 2` on, `value - 1` is at least three, so it is not itself a power of two.
  for (unsigned shift = 2; shift < 31; ++shift) {
    const thes::u32 value = thes::u32{1} << shift;
    THES_CHECK(thes::log2_floor(value) == shift);
    THES_CHECK(thes::log2_ceil(value) == shift);
    THES_CHECK(thes::log2_floor(value + 1) == shift);
    THES_CHECK(thes::log2_ceil(value + 1) == shift + 1);
    THES_CHECK(thes::log2_floor(value - 1) == shift - 1);
    THES_CHECK(thes::log2_ceil(value - 1) == shift);
  }
}

/** Checks that `bit_mask` yields the all-ones mask covering the argument’s bit width. */
THES_TEST_CASE("bit_mask covers the argument’s bit width", "[math][arithmetic]") {
  THES_CHECK(thes::bit_mask<thes::u8>(0) == 0);
  THES_CHECK(thes::bit_mask<thes::u8>(1) == 1);
  THES_CHECK(thes::bit_mask<thes::u8>(2) == 3);
  THES_CHECK(thes::bit_mask<thes::u8>(255) == 255);

  for (thes::u32 value = 1; value < 1024; ++value) {
    const thes::u32 mask = thes::bit_mask(value);
    THES_CHECK(mask >= value);
    THES_CHECK(std::popcount(mask) == std::bit_width(value));
  }
}

//==================================================================================================
// Bit manipulation
//==================================================================================================

/** Checks that `set_bit` and `get_bit` are inverse to one another at every index. */
THES_TEMPLATE_TEST_CASE("set_bit and get_bit round-trip", "[math][arithmetic]", thes::u8, thes::u16,
                        thes::u32, thes::u64) {
  static constexpr unsigned digits = std::numeric_limits<TestType>::digits;

  TestType value{};
  for (unsigned i = 0; i < digits; ++i) {
    THES_CHECK(!thes::get_bit(value, i));
    value = thes::set_bit(value, i, true);
    THES_CHECK(thes::get_bit(value, i));
  }
  THES_CHECK(value == std::numeric_limits<TestType>::max());

  for (unsigned i = 0; i < digits; ++i) {
    value = thes::set_bit(value, i, false);
    THES_CHECK(!thes::get_bit(value, i));
  }
  THES_CHECK(value == 0);

  // Setting a bit to its current value must leave the whole word untouched.
  value = thes::set_bit(TestType{}, 3U, true);
  THES_CHECK(thes::set_bit(value, 3U, true) == value);
}

/** Checks that `combine_bits` packs its arguments starting at the least significant bit. */
THES_TEST_CASE("combine_bits packs bits from the bottom up", "[math][arithmetic]") {
  THES_CHECK(thes::combine_bits<thes::u8>(false) == 0);
  THES_CHECK(thes::combine_bits<thes::u8>(true) == 1);
  THES_CHECK(thes::combine_bits<thes::u8>(false, true) == 2);
  THES_CHECK(thes::combine_bits<thes::u8>(true, true) == 3);
  THES_CHECK(thes::combine_bits<thes::u8>(true, true, true, true, true, true, true, true) == 255);
}

/** Checks the thin wrappers in `math/bit.hpp` against their `std` counterparts. */
THES_TEST_CASE("bit-width wrappers match the standard library", "[math][bit]") {
  THES_CHECK(thes::bit_width(thes::u8{0}) == 0);
  THES_CHECK(thes::countr_zero(thes::u8{0}) == 8);
  THES_CHECK(thes::countr_one(thes::u8{255}) == 8);

  for (thes::u32 value = 0; value < 512; ++value) {
    THES_CHECK(thes::bit_width(value) == unsigned(std::bit_width(value)));
    THES_CHECK(thes::countr_zero(value) == unsigned(std::countr_zero(value)));
    THES_CHECK(thes::countr_one(value) == unsigned(std::countr_one(value)));
  }
}

//==================================================================================================
// Clamped addition and subtraction
//==================================================================================================

/** Checks that `add_max`/`sub_min` clamp both at the requested bound and at the type’s range. */
THES_TEST_CASE("add_max and sub_min clamp without wrapping", "[math][arithmetic]") {
  static constexpr auto u8_max = std::numeric_limits<thes::u8>::max();

  THES_CHECK(thes::add_max<thes::u8>(1, 2, 10) == 3);
  THES_CHECK(thes::add_max<thes::u8>(1, 20, 10) == 10);
  THES_CHECK(thes::add_max<thes::u8>(200, 200, u8_max) == u8_max);
  THES_CHECK(thes::add_max<thes::u8>(200, 200, 100) == 100);

  THES_CHECK(thes::sub_min<thes::u8>(10, 2, 0) == 8);
  THES_CHECK(thes::sub_min<thes::u8>(10, 20, 0) == 0);
  THES_CHECK(thes::sub_min<thes::u8>(10, 20, 5) == 5);
  THES_CHECK(thes::sub_min<thes::u8>(10, 2, 9) == 9);
}

//==================================================================================================
// SafeInt
//==================================================================================================

/** Checks that `SafeInt` forwards its arithmetic and bitwise operators to the wrapped value. */
THES_TEST_CASE("SafeInt forwards its operators", "[math][safe-int]") {
  using Safe = thes::SafeInt<thes::i32>;

  THES_CHECK((Safe{2} + Safe{3}).unsafe() == 5);
  THES_CHECK((Safe{2} + 3).unsafe() == 5);
  THES_CHECK((Safe{7} - Safe{3}).unsafe() == 4);
  THES_CHECK((Safe{1} << 4).unsafe() == 16);
  THES_CHECK((Safe{16} >> 2).unsafe() == 4);
  THES_CHECK((Safe{0b1100} & Safe{0b1010}).unsafe() == 0b1000);
  THES_CHECK((Safe{0b1100} | Safe{0b1010}).unsafe() == 0b1110);

  Safe value{1};
  value += 4;
  THES_CHECK(value.unsafe() == 5);
  value <<= 1;
  THES_CHECK(value.unsafe() == 10);

  THES_CHECK(Safe{2} == Safe{2});
  THES_CHECK(Safe{2} < Safe{3});
  THES_CHECK(Safe{3} >= Safe{3});
}

//==================================================================================================
// FloatFraction
//==================================================================================================

/** Checks that `FloatFraction` scales only its numerator and evaluates to the quotient. */
THES_TEST_CASE("FloatFraction scales the numerator", "[math][float-fraction]") {
  const thes::FloatFraction<double> half{1.0, 2.0};
  THES_CHECK(half.to_float() == 0.5);

  const auto scaled = half * 3.0;
  THES_CHECK(scaled.numerator == 3.0);
  THES_CHECK(scaled.denominator == 2.0);
  THES_CHECK(scaled.to_float() == 1.5);

  const auto scaled_left = 4.0 * half;
  THES_CHECK(scaled_left.numerator == 4.0);
  THES_CHECK(scaled_left.to_float() == 2.0);
}

//==================================================================================================
// Constexpr-friendly cmath replacements
//==================================================================================================

static_assert(thes::cmath::abs(-3) == 3);
static_assert(thes::cmath::abs(3.5) == 3.5);
static_assert(thes::cmath::sqrt(4.0) == 2.0);
static_assert(thes::cmath::is_nan(std::numeric_limits<double>::quiet_NaN()));
static_assert(thes::cmath::is_posinf(std::numeric_limits<double>::infinity()));

/** Checks that `cmath::sqrt` agrees with `std::sqrt` and handles the special values. */
THES_TEMPLATE_TEST_CASE("cmath::sqrt matches std::sqrt", "[math][compile-time]", thes::f32,
                        thes::f64) {
  static constexpr auto eps = std::numeric_limits<TestType>::epsilon();

  THES_CHECK(thes::cmath::sqrt(TestType{0}) == TestType{0});
  THES_CHECK(thes::cmath::sqrt(TestType{1}) == TestType{1});
  THES_CHECK(thes::cmath::is_nan(thes::cmath::sqrt(TestType{-1})));
  THES_CHECK(thes::cmath::is_posinf(thes::cmath::sqrt(std::numeric_limits<TestType>::infinity())));

  for (int i = 1; i <= 200; ++i) {
    const auto value = static_cast<TestType>(i) / TestType{4};
    const auto root = thes::cmath::sqrt(value);
    const auto expected = static_cast<TestType>(std::sqrt(static_cast<double>(value)));
    THES_CHECK(thes::cmath::abs(root - expected) <= 8 * eps * expected);
  }
}

/** Checks `cmath::abs`, `is_nan` and `is_posinf` at run time, where they defer to `std`. */
THES_TEST_CASE("cmath predicates at run time", "[math][compile-time]") {
  THES_CHECK(thes::cmath::abs(-2.5) == 2.5);
  THES_CHECK(thes::cmath::abs(2.5) == 2.5);
  THES_CHECK(thes::cmath::abs(0.0) == 0.0);

  THES_CHECK(!thes::cmath::is_nan(0.0));
  THES_CHECK(thes::cmath::is_nan(std::numeric_limits<double>::quiet_NaN()));
  THES_CHECK(!thes::cmath::is_posinf(-std::numeric_limits<double>::infinity()));
  THES_CHECK(thes::cmath::is_posinf(std::numeric_limits<double>::infinity()));
}
} // namespace

THES_TEST_MAIN()
