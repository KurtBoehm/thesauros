// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <limits>
#include <stdexcept>

#include "thesauros/math/overflow.hpp"
#include "thesauros/test/test.hpp"
#include "thesauros/types/primitives.hpp"

namespace {
template<typename T>
inline constexpr T max_of = std::numeric_limits<T>::max();
template<typename T>
inline constexpr T min_of = std::numeric_limits<T>::lowest();

//==================================================================================================
// Compile-time evaluation
//==================================================================================================

static_assert(*thes::overflow_add<thes::u8>(200, 55) == 255);
static_assert(!thes::overflow_add<thes::u8>(200, 56).is_valid());
static_assert(*thes::overflow_subtract<thes::i16>(-1, 1) == -2);
static_assert(*thes::overflow_multiply<thes::u32>(1U << 15U, 1U << 16U) == (1U << 31U));
static_assert(thes::saturate_add<thes::u8>(200, 100) == 255);
static_assert(thes::saturate_subtract<thes::u8>(1, 2) == 0);
static_assert(thes::saturate_multiply<thes::u16>(300, 300) == max_of<thes::u16>);

//==================================================================================================
// Addition
//==================================================================================================

/** Checks that unsigned addition reports overflow exactly at the point of wrap-around. */
THES_TEST_CASE("overflow_add detects unsigned wrap-around", "[math][overflow]") {
  const auto fits = thes::overflow_add<thes::u8>(200, 55);
  THES_CHECK(fits.is_valid());
  THES_CHECK(*fits == 255);
  THES_CHECK(fits.valid_value() == 255);
  THES_CHECK(fits.value_or(0) == 255);

  const auto wraps = thes::overflow_add<thes::u8>(200, 56);
  THES_CHECK(!wraps.is_valid());
  THES_CHECK(wraps.raw() == 0);
  THES_CHECK(wraps.value_or(7) == 7);
  THES_CHECK_THROWS_AS(wraps.valid_value(), std::runtime_error);
}

/** Checks that signed addition reports overflow in both directions. */
THES_TEST_CASE("overflow_add detects signed overflow", "[math][overflow]") {
  THES_CHECK(thes::overflow_add<thes::i8>(100, 27).is_valid());
  THES_CHECK(!thes::overflow_add<thes::i8>(100, 28).is_valid());
  THES_CHECK(!thes::overflow_add<thes::i8>(min_of<thes::i8>, -1).is_valid());
  THES_CHECK(*thes::overflow_add<thes::i8>(-100, 100) == 0);
}

//==================================================================================================
// Subtraction and multiplication
//==================================================================================================

/** Checks that unsigned subtraction below zero is reported, while signed subtraction is not. */
THES_TEST_CASE("overflow_subtract detects underflow", "[math][overflow]") {
  THES_CHECK(*thes::overflow_subtract<thes::u16>(5, 3) == 2);
  THES_CHECK(!thes::overflow_subtract<thes::u16>(3, 5).is_valid());
  THES_CHECK(*thes::overflow_subtract<thes::i16>(3, 5) == -2);
  THES_CHECK(!thes::overflow_subtract<thes::i16>(min_of<thes::i16>, 1).is_valid());
}

/** Checks multiplication at, just below and well above the representable range. */
THES_TEST_CASE("overflow_multiply detects overflow", "[math][overflow]") {
  THES_CHECK(*thes::overflow_multiply<thes::u16>(255, 257) == max_of<thes::u16>);
  THES_CHECK(!thes::overflow_multiply<thes::u16>(300, 300).is_valid());
  THES_CHECK(*thes::overflow_multiply<thes::u64>(0, max_of<thes::u64>) == 0);
  THES_CHECK(!thes::overflow_multiply<thes::i8>(-128, -1).is_valid());
}

//==================================================================================================
// Function objects
//==================================================================================================

/** Checks that the typed and the `void` function objects agree with the free functions. */
THES_TEST_CASE("overflow function objects match the free functions", "[math][overflow]") {
  THES_CHECK(*thes::OverflowPlus<thes::u8>{}(1, 2) == 3);
  THES_CHECK(*thes::OverflowPlus<>{}(thes::u8{1}, thes::u8{2}) == 3);
  THES_CHECK(!thes::OverflowPlus<>{}(thes::u8{255}, thes::u8{1}).is_valid());

  THES_CHECK(*thes::OverflowMinus<thes::i32>{}(1, 2) == -1);
  THES_CHECK(*thes::OverflowMinus<>{}(thes::i32{1}, thes::i32{2}) == -1);
  THES_CHECK(!thes::OverflowMinus<>{}(thes::u8{1}, thes::u8{2}).is_valid());

  THES_CHECK(*thes::OverflowMultiplies<thes::u16>{}(16, 16) == 256);
  THES_CHECK(*thes::OverflowMultiplies<>{}(thes::u16{16}, thes::u16{16}) == 256);
  THES_CHECK(!thes::OverflowMultiplies<>{}(thes::u16{300}, thes::u16{300}).is_valid());
}

//==================================================================================================
// Saturating arithmetic
//==================================================================================================

/** Checks that the saturating operations clamp instead of wrapping. */
THES_TEMPLATE_TEST_CASE("saturating arithmetic clamps at the bounds", "[math][overflow]", thes::u8,
                        thes::u16, thes::u32, thes::u64) {
  static constexpr TestType max = max_of<TestType>;

  THES_CHECK(thes::saturate_add<TestType>(1, 2) == 3);
  THES_CHECK(thes::saturate_add<TestType>(max, 1) == max);
  THES_CHECK(thes::saturate_add<TestType>(max, max) == max);
  THES_CHECK(thes::saturate_add<TestType>(max, 0) == max);

  THES_CHECK(thes::saturate_subtract<TestType>(3, 2) == 1);
  THES_CHECK(thes::saturate_subtract<TestType>(2, 3) == 0);
  THES_CHECK(thes::saturate_subtract<TestType>(0, max) == 0);
  THES_CHECK(thes::saturate_subtract<TestType>(max, 0) == max);

  THES_CHECK(thes::saturate_multiply<TestType>(3, 5) == 15);
  THES_CHECK(thes::saturate_multiply<TestType>(max, 2) == max);
  THES_CHECK(thes::saturate_multiply<TestType>(max, 1) == max);
  THES_CHECK(thes::saturate_multiply<TestType>(max, 0) == 0);
}
} // namespace

THES_TEST_MAIN()
