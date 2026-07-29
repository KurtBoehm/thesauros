// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <bit>
#include <climits>
#include <concepts>
#include <cstddef>
#include <expected>
#include <functional>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "thesauros/test/test.hpp"
#include "thesauros/types/primitives.hpp"
#include "thesauros/types/value-tag.hpp"
#include "thesauros/utility/arrow-proxy.hpp"
#include "thesauros/utility/as-expected.hpp"
#include "thesauros/utility/byte-integer.hpp"
#include "thesauros/utility/integral-value.hpp"
#include "thesauros/utility/kind-convert.hpp"
#include "thesauros/utility/move-detector.hpp"
#include "thesauros/utility/store-type.hpp"
#include "thesauros/utility/unwrap.hpp"

namespace {
struct Point {
  int x{};
  int y{};

  [[nodiscard]] int sum() const {
    return x + y;
  }
};

//==================================================================================================
// ArrowProxy
//==================================================================================================

static_assert(
  std::same_as<decltype(thes::ArrowCreator<int, int*>::create(std::declval<int&>())), int*>);
static_assert(
  std::same_as<decltype(thes::ArrowCreator<int, const int*>::create(std::declval<const int&>())),
               const int*>);

/** Checks that `ArrowProxy` keeps a value alive so that `operator->` can reach into it. */
THES_TEST_CASE("ArrowProxy owns the value it points at", "[utility][arrow-proxy]") {
  thes::ArrowProxy<Point> proxy{Point{1, 2}};

  THES_CHECK(proxy->x == 1);
  THES_CHECK(proxy->y == 2);
  THES_CHECK(proxy->sum() == 3);
  THES_CHECK(proxy.operator->() == &proxy.value);

  // The arrow yields a mutable pointer into the stored value.
  proxy->x = 10;
  THES_CHECK(proxy.value.x == 10);
}

/** Checks the three `ArrowCreator` specializations, which pick between a pointer and a proxy. */
THES_TEST_CASE("ArrowCreator selects a pointer or a proxy", "[utility][arrow-proxy]") {
  Point point{.x = 3, .y = 4};

  int* const mutable_ptr = thes::ArrowCreator<int, int*>::create(point.x);
  THES_CHECK(mutable_ptr == &point.x);

  const int* const const_ptr = thes::ArrowCreator<int, const int*>::create(point.y);
  THES_CHECK(const_ptr == &point.y);

  // A prvalue has to be stored somewhere, which is what the proxy is for.
  auto proxy = thes::ArrowCreator<Point, thes::ArrowProxy<Point>>::create(Point{5, 6});
  static_assert(std::same_as<decltype(proxy), thes::ArrowProxy<Point>>);
  THES_CHECK(proxy->sum() == 11);
}

//==================================================================================================
// MoveDetector
//==================================================================================================

static_assert(!std::copy_constructible<thes::MoveDetector>);
static_assert(std::move_constructible<thes::MoveDetector>);

/** Checks that a detector notices being moved from, and that the target stays unmoved. */
THES_TEST_CASE("MoveDetector records a move", "[utility][move-detector]") {
  thes::MoveDetector source{};
  THES_CHECK(source.is_unmoved());
  THES_CHECK(!source.is_moved());

  const thes::MoveDetector target{std::move(source)};
  THES_CHECK(target.is_unmoved());
  THES_CHECK(source.is_moved());
  THES_CHECK(!source.is_unmoved());
}

/** Checks that the moved-from state propagates along a chain of moves. */
THES_TEST_CASE("a move chain leaves every source moved", "[utility][move-detector]") {
  thes::MoveDetector first{};
  thes::MoveDetector second{std::move(first)};
  const thes::MoveDetector third{std::move(second)};

  THES_CHECK(first.is_moved());
  THES_CHECK(second.is_moved());
  THES_CHECK(third.is_unmoved());
}

//==================================================================================================
// StoreType
//==================================================================================================

static_assert(std::same_as<thes::StoreType<int>, int>);
static_assert(std::same_as<thes::StoreType<int&&>, int>);
// An lvalue reference cannot be stored as a member, so it becomes a pointer.
static_assert(std::same_as<thes::StoreType<int&>, int*>);
static_assert(std::same_as<thes::StoreType<const int&>, const int*>);

/** Checks that a value is stored by copy and comes back as a reference to that copy. */
THES_TEST_CASE("values are stored by copy", "[utility][store-type]") {
  int original = 3;
  auto stored = thes::to_stored(std::move(original));
  static_assert(std::same_as<decltype(stored), int>);

  THES_CHECK(stored == 3);
  THES_CHECK(thes::from_stored<int>(stored) == 3);

  // Writing through the retrieved reference changes the stored copy, not the source.
  thes::from_stored<int>(stored) = 7;
  THES_CHECK(stored == 7);
  THES_CHECK(original == 3);
}

/** Checks that an lvalue is stored as a pointer and comes back as a reference to the original. */
THES_TEST_CASE("lvalues are stored by address", "[utility][store-type]") {
  int original = 3;
  auto stored = thes::to_stored(original);
  static_assert(std::same_as<decltype(stored), int*>);

  THES_CHECK(stored == &original);
  THES_CHECK(thes::from_stored<int&>(stored) == 3);

  // Writing through the retrieved reference reaches the original object.
  thes::from_stored<int&>(stored) = 7;
  THES_CHECK(original == 7);
}

/** Checks the const overload of `from_stored`. */
THES_TEST_CASE("from_stored has a const overload", "[utility][store-type]") {
  int value = 5;
  const auto stored_ref = thes::to_stored(value);
  const auto stored_val = thes::to_stored(int{9});

  static_assert(std::same_as<decltype(thes::from_stored<int&>(stored_ref)), const int&>);
  static_assert(std::same_as<decltype(thes::from_stored<int>(stored_val)), const int&>);

  THES_CHECK(thes::from_stored<int&>(stored_ref) == 5);
  THES_CHECK(thes::from_stored<int>(stored_val) == 9);
}

//==================================================================================================
// unwrap and as_reference
//==================================================================================================

/** Checks that `unwrap` unpacks a `std::reference_wrapper` and passes anything else through. */
THES_TEST_CASE("unwrap unpacks reference wrappers", "[utility][unwrap]") {
  int value = 3;

  auto& unwrapped = thes::unwrap(std::ref(value));
  static_assert(std::same_as<decltype(unwrapped), int&>);
  THES_CHECK(&unwrapped == &value);

  unwrapped = 7;
  THES_CHECK(value == 7);

  // A plain lvalue is forwarded unchanged.
  static_assert(std::same_as<decltype(thes::unwrap(value)), int&>);
  THES_CHECK(&thes::unwrap(value) == &value);

  // A `cref` wrapper unwraps to a reference to const.
  const auto& const_unwrapped = thes::unwrap(std::cref(value));
  static_assert(std::same_as<decltype(const_unwrapped), const int&>);
  THES_CHECK(const_unwrapped == 7);
}

/** Checks that `as_reference` binds an lvalue reference without copying. */
THES_TEST_CASE("as_reference does not copy", "[utility][kind-convert]") {
  std::vector<int> values{1, 2, 3};

  auto& ref = thes::as_reference(values);
  static_assert(std::same_as<decltype(ref), std::vector<int>&>);
  THES_CHECK(&ref == &values);

  ref.push_back(4);
  THES_CHECK(values.size() == 4);

  const std::string text{"abc"};
  static_assert(std::same_as<decltype(thes::as_reference(text)), const std::string&>);
  THES_CHECK(&thes::as_reference(text) == &text);
}

//==================================================================================================
// integral_value
//==================================================================================================

static_assert(std::same_as<thes::IntegralValue<int>, int>);
static_assert(std::same_as<thes::IntegralValue<thes::IndexTag<3>>, std::size_t>);
static_assert(thes::integral_value(thes::index_tag<3>) == 3);
static_assert(thes::integral_value(7) == 7);

/** Checks that `integral_value` erases a `ValueTag` down to its underlying value. */
THES_TEST_CASE("integral_value unwraps value tags", "[utility][integral-value]") {
  THES_CHECK(thes::integral_value(thes::u32{5}) == 5);
  THES_CHECK(thes::integral_value(thes::index_tag<0>) == 0);
  THES_CHECK(thes::integral_value(thes::index_tag<42>) == 42);

  // The result is an ordinary run-time value of the tag’s underlying type.
  const auto value = thes::integral_value(thes::index_tag<9>);
  static_assert(std::same_as<decltype(value), const std::size_t>);
  THES_CHECK(value == 9);
}

//==================================================================================================
// as_expected
//==================================================================================================

/** Checks that a zero return code becomes a value and anything else an error. */
THES_TEST_CASE("as_expected maps C-style return codes", "[utility][as-expected]") {
  const auto ok = thes::as_expected(0);
  THES_CHECK(ok.has_value());

  const auto failed = thes::as_expected(-1);
  THES_CHECK(!failed.has_value());
  THES_CHECK(failed.error() == -1);

  const auto errno_like = thes::as_expected(2);
  THES_CHECK(!errno_like.has_value());
  THES_CHECK(errno_like.error() == 2);

  static_assert(
    std::same_as<decltype(thes::as_expected(thes::i64{0})), std::expected<void, thes::i64>>);
}

//==================================================================================================
// ByteInteger
//==================================================================================================

static_assert(std::same_as<thes::ByteInteger<1>::Unsigned, thes::u8>);
static_assert(std::same_as<thes::ByteInteger<2>::Unsigned, thes::u16>);
// Three bytes are held in the next power-of-two-sized type, leaving one byte of overhead.
static_assert(std::same_as<thes::ByteInteger<3>::Unsigned, thes::u32>);
static_assert(std::same_as<thes::ByteInteger<4>::Unsigned, thes::u32>);
static_assert(std::same_as<thes::ByteInteger<8>::Unsigned, thes::u64>);

static_assert(thes::ByteInteger<3>::overhead_byte_num == 1);
static_assert(thes::ByteInteger<4>::overhead_byte_num == 0);
static_assert(thes::ByteInteger<5>::overhead_byte_num == 3);

/** Checks that `max` is the largest value representable in `byte_num` bytes. */
THES_TEMPLATE_TEST_CASE("ByteInteger reports the packed value range", "[utility][byte-integer]",
                        thes::ValueTag<std::size_t, 1>, thes::ValueTag<std::size_t, 2>,
                        thes::ValueTag<std::size_t, 3>, thes::ValueTag<std::size_t, 4>,
                        thes::ValueTag<std::size_t, 5>, thes::ValueTag<std::size_t, 8>) {
  using Int = thes::ByteInteger<TestType::value>;
  using Unsigned = Int::Unsigned;

  THES_CHECK(Int::byte_num == TestType::value);
  THES_CHECK(Int::bit_num == CHAR_BIT * TestType::value);
  THES_CHECK(sizeof(Unsigned) >= Int::byte_num);
  THES_CHECK(Int::overhead_byte_num == sizeof(Unsigned) - Int::byte_num);
  THES_CHECK(Int::overhead_bit_num == CHAR_BIT * Int::overhead_byte_num);

  // `max` has exactly `bit_num` bits set, so it is the largest packed value.
  THES_CHECK(std::popcount(Int::max) == Int::bit_num);
  if (Int::overhead_bit_num > 0) {
    THES_CHECK(Int::max < std::numeric_limits<Unsigned>::max());
  } else {
    THES_CHECK(Int::max == std::numeric_limits<Unsigned>::max());
  }
}
} // namespace

THES_TEST_MAIN()
