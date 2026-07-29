// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <limits>
#include <stdexcept>

#include "thesauros/math/integer-cast.hpp"
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

static_assert(*thes::safe_cast<thes::u8>(255) == 255);
static_assert(!thes::safe_cast<thes::u8>(256).is_valid());
static_assert(!thes::safe_cast<thes::u8>(-1).is_valid());
static_assert(thes::saturate_cast<thes::u8>(256) == 255);
static_assert(thes::saturate_cast<thes::i8>(-1000) == -128);

//==================================================================================================
// safe_cast
//==================================================================================================

/** Checks that a narrowing cast to an unsigned type reports both bounds. */
THES_TEST_CASE("safe_cast to a narrower unsigned type", "[math][integer-cast]") {
  THES_CHECK(thes::safe_cast<thes::u8>(0).is_valid());
  THES_CHECK(*thes::safe_cast<thes::u8>(255) == 255);
  THES_CHECK(!thes::safe_cast<thes::u8>(256).is_valid());
  THES_CHECK(!thes::safe_cast<thes::u8>(-1).is_valid());
  THES_CHECK(!thes::safe_cast<thes::u8>(min_of<int>).is_valid());
  THES_CHECK(!thes::safe_cast<thes::u8>(max_of<int>).is_valid());
}

/** Checks that a narrowing cast to a signed type reports both bounds. */
THES_TEST_CASE("safe_cast to a narrower signed type", "[math][integer-cast]") {
  THES_CHECK(*thes::safe_cast<thes::i8>(127) == 127);
  THES_CHECK(*thes::safe_cast<thes::i8>(-128) == -128);
  THES_CHECK(!thes::safe_cast<thes::i8>(128).is_valid());
  THES_CHECK(!thes::safe_cast<thes::i8>(-129).is_valid());
}

/** Checks the sign-changing casts, where only one of the two bounds can be violated. */
THES_TEST_CASE("safe_cast across signedness", "[math][integer-cast]") {
  // Every non-negative `i64` fits into `u64`, and only negative values do not.
  THES_CHECK(*thes::safe_cast<thes::u64>(thes::i64{0}) == 0);
  THES_CHECK(*thes::safe_cast<thes::u64>(max_of<thes::i64>) == thes::u64{max_of<thes::i64>});
  THES_CHECK(!thes::safe_cast<thes::u64>(thes::i64{-1}).is_valid());

  // Conversely, only `u64` values above `i64`’s maximum do not fit into `i64`.
  THES_CHECK(*thes::safe_cast<thes::i64>(thes::u64{0}) == 0);
  THES_CHECK(*thes::safe_cast<thes::i64>(thes::u64{max_of<thes::i64>}) == max_of<thes::i64>);
  THES_CHECK(!thes::safe_cast<thes::i64>(thes::u64{max_of<thes::i64>} + 1).is_valid());
  THES_CHECK(!thes::safe_cast<thes::i64>(max_of<thes::u64>).is_valid());
}

/** Checks that widening casts, which can never fail, preserve the value. */
THES_TEST_CASE("safe_cast widening preserves the value", "[math][integer-cast]") {
  THES_CHECK(*thes::safe_cast<thes::i64>(min_of<thes::i32>) == min_of<thes::i32>);
  THES_CHECK(*thes::safe_cast<thes::u64>(max_of<thes::u32>) == max_of<thes::u32>);
  THES_CHECK(*thes::safe_cast<thes::i64>(max_of<thes::u32>) == max_of<thes::u32>);
  THES_CHECK(*thes::safe_cast<thes::u32>(max_of<thes::u32>) == max_of<thes::u32>);
}

/** Checks the `InfoResult` accessors on an out-of-range result. */
THES_TEST_CASE("safe_cast exposes the truncated value and throws on demand",
               "[math][integer-cast]") {
  const auto result = thes::safe_cast<thes::u8>(258);
  THES_CHECK(!result.is_valid());
  THES_CHECK(result.raw() == 2);
  THES_CHECK(result.value_or(9) == 9);
  THES_CHECK_THROWS_AS(result.valid_value(), std::runtime_error);
  THES_CHECK_NOTHROW(thes::safe_cast<thes::u8>(250).valid_value());
}

//==================================================================================================
// saturate_cast
//==================================================================================================

/** Checks that `saturate_cast` clamps rather than wrapping. */
THES_TEST_CASE("saturate_cast clamps to the target range", "[math][integer-cast]") {
  THES_CHECK(thes::saturate_cast<thes::u8>(300) == 255);
  THES_CHECK(thes::saturate_cast<thes::u8>(-300) == 0);
  THES_CHECK(thes::saturate_cast<thes::u8>(42) == 42);

  THES_CHECK(thes::saturate_cast<thes::i8>(300) == 127);
  THES_CHECK(thes::saturate_cast<thes::i8>(-300) == -128);
  THES_CHECK(thes::saturate_cast<thes::i8>(-42) == -42);

  THES_CHECK(thes::saturate_cast<thes::u64>(thes::i64{-1}) == 0);
  THES_CHECK(thes::saturate_cast<thes::i64>(max_of<thes::u64>) == max_of<thes::i64>);
}

/** Checks that `saturate_cast` agrees with `safe_cast` whenever the latter succeeds. */
THES_TEMPLATE_TEST_CASE("saturate_cast agrees with safe_cast in range", "[math][integer-cast]",
                        thes::i8, thes::u8, thes::i16, thes::u16) {
  for (int i = -1000; i <= 1000; ++i) {
    const auto safe = thes::safe_cast<TestType>(i);
    if (safe.is_valid()) {
      THES_CHECK(thes::saturate_cast<TestType>(i) == *safe);
    } else {
      const TestType bound =
        (i < 0) ? std::numeric_limits<TestType>::lowest() : std::numeric_limits<TestType>::max();
      THES_CHECK(thes::saturate_cast<TestType>(i) == bound);
    }
  }
}
} // namespace

THES_TEST_MAIN()
