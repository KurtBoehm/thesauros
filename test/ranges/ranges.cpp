// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <numeric>
#include <ranges>
#include <tuple>

#include "thesauros/format.hpp"
#include "thesauros/ranges/enumerate.hpp"
#include "thesauros/ranges/indices.hpp"
#include "thesauros/test/equality.hpp"
#include "thesauros/test/test.hpp"

namespace test = thes::test;

namespace {
//==================================================================================================
// indices
//==================================================================================================

constexpr auto ints = thes::views::indices(10);
static_assert(ints.contains(9));

/** Checks the traversal, indexing and formatting of `indices`. */
THES_TEST_CASE("indices covers a half-open range", "[ranges][indices]") {
  THES_CHECK(fmt::format("{}", ints) == "[0, 10)");
  THES_CHECK(ints.contains(9));
  THES_CHECK(!ints.contains(10));

  auto it = ints.begin();
  THES_CHECK(*(++it) == 1);
  it = ints.begin();
  THES_CHECK(*(it++) == 0);
  THES_CHECK(*it == 1);

  THES_CHECK(ints.begin()[4] == 4);
  THES_CHECK(*(ints.begin() + 4) == 4);
  THES_CHECK(std::reduce(ints.begin(), ints.end()) == 45);
}

constexpr auto reversed_ints = std::views::reverse(thes::views::indices(10));

/** Checks that reversing `indices` walks the same values back to front. */
THES_TEST_CASE("reversed indices walk backwards", "[ranges][indices]") {
  THES_CHECK(fmt::format("{}", reversed_ints) == "[9, 8, 7, 6, 5, 4, 3, 2, 1, 0]");

  auto it = reversed_ints.begin();
  THES_CHECK(*(++it) == 8);
  it = reversed_ints.begin();
  THES_CHECK(*(it++) == 9);
  THES_CHECK(*it == 8);

  THES_CHECK(reversed_ints.begin()[4] == 5);
  THES_CHECK(*(reversed_ints.begin() + 4) == 5);
  THES_CHECK(std::reduce(reversed_ints.begin(), reversed_ints.end()) == 45);
}

//==================================================================================================
// Composition with the standard views
//==================================================================================================

constexpr auto doubled =
  thes::views::indices(10) | std::views::transform([](auto v) { return 2 * v; });
static_assert(test::range_eq(doubled, std::array{0, 2, 4, 6, 8, 10, 12, 14, 16, 18}));

/** Checks that `indices` can be transformed, indexed and folded. */
THES_TEST_CASE("indices compose with transform", "[ranges][indices]") {
  THES_CHECK(test::range_eq(doubled, std::array{0, 2, 4, 6, 8, 10, 12, 14, 16, 18}));
  THES_CHECK(doubled.begin()[1] == 2);
  THES_CHECK(std::ranges::fold_left(doubled, 0, std::plus{}) == 90);
}

/** Checks that a subrange over a transformed `indices` behaves like the transformed range. */
THES_TEST_CASE("a subrange preserves the transformed range", "[ranges][indices]") {
  constexpr auto sub = std::ranges::subrange{doubled.begin(), doubled.end()};

  THES_CHECK(test::range_eq(sub, std::array{0, 2, 4, 6, 8, 10, 12, 14, 16, 18}));
  THES_CHECK(sub.begin()[1] == 2);
}

//==================================================================================================
// enumerate
//==================================================================================================

/** Checks that `enumerate` pairs every element with its index. */
THES_TEST_CASE("enumerate pairs elements with their indices", "[ranges][enumerate]") {
  static constexpr std::array base{8, 4, 6, 2};
  static constexpr auto enumerated = thes::views::enumerate<std::size_t>(base);
  using Value = decltype(enumerated)::Value;

  THES_CHECK(test::range_eq(enumerated, std::array{Value{0, base[0]}, Value{1, base[1]},
                                                   Value{2, base[2]}, Value{3, base[3]}}));
}

//==================================================================================================
// zip
//==================================================================================================

/** Checks that `zip` pairs the elements of two ranges. */
THES_TEST_CASE("zip pairs corresponding elements", "[ranges][zip]") {
  constexpr std::array v1{8, 4};
  constexpr std::array v2{6, 2};
  const auto zipped = std::views::zip(v1, v2);

  THES_CHECK(zipped.size() == 2);
  THES_CHECK(
    test::range_eq(zipped, std::array{std::tuple{v1[0], v2[0]}, std::tuple{v1[1], v2[1]}}));
}
} // namespace

THES_TEST_MAIN()
