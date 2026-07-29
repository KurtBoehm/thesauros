// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <concepts>
#include <limits>
#include <string>

#include "thesauros/test/test.hpp"
#include "thesauros/types/primitives.hpp"
#include "thesauros/types/type-transformations.hpp"

namespace {
struct Aggregate {
  int number;
  double weight;
  std::string name;
};

//==================================================================================================
// MemberType
//==================================================================================================

static_assert(std::same_as<thes::MemberType<decltype(&Aggregate::number)>, int>);
static_assert(std::same_as<thes::MemberType<decltype(&Aggregate::weight)>, double>);
static_assert(std::same_as<thes::MemberType<decltype(&Aggregate::name)>, std::string>);

//==================================================================================================
// Const transformations
//==================================================================================================

static_assert(std::same_as<thes::AddConst<int>, const int>);
static_assert(std::same_as<thes::AddConst<const int>, const int>);
// Unlike `std::add_const`, this adds the `const` below the reference, where it has an effect.
static_assert(std::same_as<thes::AddConst<int&>, const int&>);
static_assert(std::same_as<thes::AddConst<int*>, int* const>);

static_assert(std::same_as<thes::ConditionalConst<true, int>, const int>);
static_assert(std::same_as<thes::ConditionalConst<false, int>, int>);

static_assert(std::same_as<thes::TransferConst<const double, int>, const int>);
static_assert(std::same_as<thes::TransferConst<double, int>, int>);
// Only top-level constness is inspected, so a reference to const does not transfer.
static_assert(std::same_as<thes::TransferConst<const double&, int>, int>);

static_assert(std::same_as<thes::TransferConstAccess<const double&, int>, const int>);
static_assert(std::same_as<thes::TransferConstAccess<double&, int>, int>);
static_assert(std::same_as<thes::TransferConstAccess<const double, int>, const int>);

//==================================================================================================
// First
//==================================================================================================

static_assert(std::same_as<thes::First<int, double>, int>);
static_assert(std::same_as<thes::First<int, void>, int>);

//==================================================================================================
// Union: the smallest type holding both operands
//==================================================================================================

static_assert(std::same_as<thes::Union<int>, int>);
static_assert(std::same_as<thes::Union<int, int>, int>);

// Same signedness: the wider type wins.
static_assert(std::same_as<thes::Union<thes::u8, thes::u32>, thes::u32>);
static_assert(std::same_as<thes::Union<thes::u64, thes::u16>, thes::u64>);
static_assert(std::same_as<thes::Union<thes::i8, thes::i32>, thes::i32>);
static_assert(std::same_as<thes::Union<thes::i64, thes::i16>, thes::i64>);

// Mixed signedness: a strictly wider signed type can hold the unsigned one as it is; otherwise the
// unsigned type has to be widened to twice its size to stay representable.
static_assert(std::same_as<thes::Union<thes::u8, thes::i16>, thes::i16>);
static_assert(std::same_as<thes::Union<thes::u8, thes::i8>, thes::i16>);
static_assert(std::same_as<thes::Union<thes::u16, thes::i16>, thes::i32>);
static_assert(std::same_as<thes::Union<thes::u32, thes::i16>, thes::i64>);
// The operand order does not matter.
static_assert(std::same_as<thes::Union<thes::i16, thes::u8>, thes::i16>);
static_assert(std::same_as<thes::Union<thes::i16, thes::u16>, thes::i32>);

static_assert(std::same_as<thes::Union<thes::f32, thes::f64>, thes::f64>);
static_assert(std::same_as<thes::Union<thes::f64, thes::f32>, thes::f64>);

// More than two operands fold from the left.
static_assert(std::same_as<thes::Union<thes::u8, thes::u16, thes::u32>, thes::u32>);
static_assert(std::same_as<thes::Union<thes::u8, thes::u8, thes::u8>, thes::u8>);
static_assert(std::same_as<thes::Union<thes::i8, thes::u8, thes::i64>, thes::i64>);

//==================================================================================================
// Intersection: the largest type both operands can hold
//==================================================================================================

static_assert(std::same_as<thes::Intersection<int>, int>);
static_assert(std::same_as<thes::Intersection<int, int>, int>);

static_assert(std::same_as<thes::Intersection<thes::u8, thes::u32>, thes::u8>);
static_assert(std::same_as<thes::Intersection<thes::u64, thes::u16>, thes::u16>);
static_assert(std::same_as<thes::Intersection<thes::i8, thes::i32>, thes::i8>);
static_assert(std::same_as<thes::Intersection<thes::f32, thes::f64>, thes::f32>);

static_assert(std::same_as<thes::Intersection<thes::u32, thes::u16, thes::u8>, thes::u8>);
static_assert(std::same_as<thes::Intersection<thes::i64, thes::i64, thes::i16>, thes::i16>);

/**
 * Checks at run time that `Union` really can represent both operands’ extremes, which is the
 * property the trait exists for.
 */
THES_TEST_CASE("a union type holds both operands’ ranges", "[types][type-transformations]") {
  {
    using U = thes::Union<thes::u8, thes::i8>;
    THES_CHECK(std::numeric_limits<U>::max() >= std::numeric_limits<thes::u8>::max());
    THES_CHECK(std::numeric_limits<U>::lowest() <= std::numeric_limits<thes::i8>::lowest());
  }
  {
    using U = thes::Union<thes::u16, thes::i16>;
    THES_CHECK(std::numeric_limits<U>::max() >= std::numeric_limits<thes::u16>::max());
    THES_CHECK(std::numeric_limits<U>::lowest() <= std::numeric_limits<thes::i16>::lowest());
  }
  {
    using U = thes::Union<thes::u8, thes::u32>;
    THES_CHECK(std::numeric_limits<U>::max() >= std::numeric_limits<thes::u32>::max());
  }
}

/** Checks that an intersection type’s range is contained in both operands’ ranges. */
THES_TEST_CASE("an intersection type fits into both operands", "[types][type-transformations]") {
  using I = thes::Intersection<thes::u8, thes::u32>;
  THES_CHECK(std::numeric_limits<I>::max() <= std::numeric_limits<thes::u8>::max());
  THES_CHECK(std::numeric_limits<I>::max() <= std::numeric_limits<thes::u32>::max());
}

/** Checks that `AddConst` on a reference really yields a read-only view of the referee. */
THES_TEST_CASE("AddConst on a reference protects the referee", "[types][type-transformations]") {
  int value = 3;
  thes::AddConst<int&> ref = value;
  THES_CHECK(ref == 3);

  value = 4;
  THES_CHECK(ref == 4);
  static_assert(!std::assignable_from<decltype(ref), int>);
}
} // namespace

THES_TEST_MAIN()
