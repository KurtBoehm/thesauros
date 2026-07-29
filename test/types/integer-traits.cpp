// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <climits>
#include <concepts>
#include <limits>

#include "thesauros/test/test.hpp"
#include "thesauros/types/fixed-size-integer.hpp"
#include "thesauros/types/numeric-info.hpp"
#include "thesauros/types/primitives.hpp"
#include "thesauros/types/signedness.hpp"

namespace {
//==================================================================================================
// FixedIntTrait
//==================================================================================================

static_assert(std::same_as<thes::FixedUnsignedInt<1>, thes::u8>);
static_assert(std::same_as<thes::FixedUnsignedInt<2>, thes::u16>);
static_assert(std::same_as<thes::FixedUnsignedInt<4>, thes::u32>);
static_assert(std::same_as<thes::FixedUnsignedInt<8>, thes::u64>);

static_assert(std::same_as<thes::FixedSignedInt<1>, thes::i8>);
static_assert(std::same_as<thes::FixedSignedInt<2>, thes::i16>);
static_assert(std::same_as<thes::FixedSignedInt<4>, thes::i32>);
static_assert(std::same_as<thes::FixedSignedInt<8>, thes::i64>);

// Sizes that are not powers of two round up to the next one, via `std::bit_ceil`.
static_assert(std::same_as<thes::FixedUnsignedInt<3>, thes::u32>);
static_assert(std::same_as<thes::FixedUnsignedInt<5>, thes::u64>);
static_assert(std::same_as<thes::FixedUnsignedInt<6>, thes::u64>);
static_assert(std::same_as<thes::FixedUnsignedInt<7>, thes::u64>);
static_assert(std::same_as<thes::FixedSignedInt<3>, thes::i32>);

/** Checks that each fixed-size integer really occupies the number of bytes it is named for. */
THES_TEST_CASE("fixed-size integers have the requested width", "[types][fixed-size-integer]") {
  THES_CHECK(sizeof(thes::FixedUnsignedInt<1>) == 1);
  THES_CHECK(sizeof(thes::FixedUnsignedInt<2>) == 2);
  THES_CHECK(sizeof(thes::FixedUnsignedInt<4>) == 4);
  THES_CHECK(sizeof(thes::FixedUnsignedInt<8>) == 8);
  THES_CHECK(sizeof(thes::FixedSignedInt<8>) == 8);

  // A rounded-up size is at least as wide as requested, which is what callers rely on.
  THES_CHECK(sizeof(thes::FixedUnsignedInt<3>) >= 3);
  THES_CHECK(sizeof(thes::FixedUnsignedInt<5>) >= 5);
}

//==================================================================================================
// Signedness
//==================================================================================================

static_assert(std::same_as<thes::MakeSigned<thes::u8>, thes::i8>);
static_assert(std::same_as<thes::MakeSigned<thes::i8>, thes::i8>);
static_assert(std::same_as<thes::MakeUnsigned<thes::i8>, thes::u8>);
static_assert(std::same_as<thes::MakeUnsigned<thes::u8>, thes::u8>);

static_assert(std::same_as<thes::MakeSigned<thes::u64>, thes::i64>);
static_assert(std::same_as<thes::MakeUnsigned<thes::i64>, thes::u64>);

// The 128-bit types need their own specializations, since `std::make_signed` need not cover them.
static_assert(std::same_as<thes::MakeSigned<thes::u128>, thes::i128>);
static_assert(std::same_as<thes::MakeSigned<thes::i128>, thes::i128>);
static_assert(std::same_as<thes::MakeUnsigned<thes::i128>, thes::u128>);
static_assert(std::same_as<thes::MakeUnsigned<thes::u128>, thes::u128>);

/** Checks that changing signedness preserves the width. */
THES_TEMPLATE_TEST_CASE("changing signedness preserves the width", "[types][signedness]", thes::u8,
                        thes::u16, thes::u32, thes::u64, thes::i8, thes::i16, thes::i32,
                        thes::i64) {
  THES_CHECK(sizeof(thes::MakeSigned<TestType>) == sizeof(TestType));
  THES_CHECK(sizeof(thes::MakeUnsigned<TestType>) == sizeof(TestType));
  THES_CHECK(std::signed_integral<thes::MakeSigned<TestType>>);
  THES_CHECK(std::unsigned_integral<thes::MakeUnsigned<TestType>>);
}

//==================================================================================================
// NumericInfo
//==================================================================================================

static_assert(thes::NumericInfo<thes::u8>::byte_num == 1);
static_assert(thes::NumericInfo<thes::u8>::bit_num == 8);
static_assert(thes::NumericInfo<thes::u8>::digits == 8);
static_assert(thes::NumericInfo<thes::u8>::max == 255);

static_assert(thes::NumericInfo<thes::i8>::byte_num == 1);
static_assert(thes::NumericInfo<thes::i8>::bit_num == 8);
// A signed type spends one bit on the sign, so it has one digit less than its width.
static_assert(thes::NumericInfo<thes::i8>::digits == 7);
static_assert(thes::NumericInfo<thes::i8>::max == 127);

/** Checks `NumericInfo` against `sizeof` and `std::numeric_limits`. */
THES_TEMPLATE_TEST_CASE("NumericInfo mirrors the standard traits", "[types][numeric-info]",
                        thes::u8, thes::u16, thes::u32, thes::u64, thes::i8, thes::i16, thes::i32,
                        thes::i64, thes::f32, thes::f64) {
  using Info = thes::NumericInfo<TestType>;

  THES_CHECK(Info::byte_num == sizeof(TestType));
  THES_CHECK(Info::bit_num == CHAR_BIT * sizeof(TestType));
  THES_CHECK(Info::digits == unsigned(std::numeric_limits<TestType>::digits));
  THES_CHECK(Info::max == std::numeric_limits<TestType>::max());
}
} // namespace

THES_TEST_MAIN()
