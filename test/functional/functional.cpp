// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <concepts>
#include <numeric>
#include <string>
#include <vector>

#include "thesauros/functional/binary.hpp"
#include "thesauros/functional/no-op.hpp"
#include "thesauros/test/test.hpp"

namespace {
//==================================================================================================
// Maximum and Minimum
//==================================================================================================

static_assert(thes::Maximum{}(2, 3) == 3);
static_assert(thes::Minimum{}(2, 3) == 2);
static_assert(thes::Maximum{}(3, 3) == 3);

/** Checks that the function objects agree with `std::max`/`std::min`, including on ties. */
THES_TEST_CASE("Maximum and Minimum select the extreme argument", "[functional][binary]") {
  THES_CHECK(thes::Maximum{}(1, 2) == 2);
  THES_CHECK(thes::Maximum{}(2, 1) == 2);
  THES_CHECK(thes::Minimum{}(1, 2) == 1);
  THES_CHECK(thes::Minimum{}(2, 1) == 1);

  THES_CHECK(thes::Maximum{}(-1.5, 2.5) == 2.5);
  THES_CHECK(thes::Minimum{}(-1.5, 2.5) == -1.5);

  const std::string first{"abc"};
  const std::string second{"abd"};
  THES_CHECK(thes::Maximum{}(first, second) == second);
  THES_CHECK(thes::Minimum{}(first, second) == first);
}

/** Checks that they work as the binary operation of a fold, which is their intended use. */
THES_TEST_CASE("Maximum and Minimum work as fold operations", "[functional][binary]") {
  const std::vector<int> values{3, 1, 4, 1, 5, 9, 2, 6};

  THES_CHECK(std::accumulate(values.begin(), values.end(), values.front(), thes::Maximum{}) == 9);
  THES_CHECK(std::accumulate(values.begin(), values.end(), values.front(), thes::Minimum{}) == 1);

  const std::vector<int> empty{};
  THES_CHECK(std::accumulate(empty.begin(), empty.end(), 7, thes::Maximum{}) == 7);
}

//==================================================================================================
// NoOp
//==================================================================================================

static_assert(std::same_as<decltype(thes::NoOp{}), thes::NoOp<void>>);
static_assert(thes::AnyNoOp<thes::NoOp<void>>);
static_assert(thes::AnyNoOp<thes::NoOp<int>>);
static_assert(!thes::AnyNoOp<thes::Maximum>);
static_assert(!thes::AnyNoOp<int>);

static_assert(thes::NoOp<int>{}() == 0);
static_assert(thes::NoOp<int>{7}() == 7);

/** Checks that the `void` specialization accepts and ignores any arguments. */
THES_TEST_CASE("NoOp<void> ignores its arguments", "[functional][no-op]") {
  const thes::NoOp<void> op{};
  static_assert(std::same_as<decltype(op()), void>);

  op();
  op(1);
  op(1, "two", 3.0);
  THES_CHECK(true);
}

/** Checks that the value-returning specialization always returns the stored value. */
THES_TEST_CASE("NoOp<T> returns its stored value", "[functional][no-op]") {
  const thes::NoOp<int> zero{};
  THES_CHECK(zero() == 0);
  THES_CHECK(zero(1, 2, 3) == 0);

  const thes::NoOp<int> seven{7};
  THES_CHECK(seven() == 7);
  THES_CHECK(seven("ignored") == 7);

  const thes::NoOp<bool> yes{true};
  THES_CHECK(yes());
  THES_CHECK(yes(1, 2));
}

/** Checks that `AnyNoOp` is what callers use to compile out optional work. */
THES_TEST_CASE("AnyNoOp distinguishes the no-op types", "[functional][no-op]") {
  const auto is_no_op = []<typename T>() { return thes::AnyNoOp<T>; };

  THES_CHECK(is_no_op.operator()<thes::NoOp<void>>());
  THES_CHECK(is_no_op.operator()<thes::NoOp<double>>());
  THES_CHECK(!is_no_op.operator()<thes::Minimum>());
  THES_CHECK(!is_no_op.operator()<std::string>());
}
} // namespace

THES_TEST_MAIN()
