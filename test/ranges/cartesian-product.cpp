// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <functional>
#include <iterator>
#include <ranges>
#include <tuple>

#include "thesauros/format.hpp"
#include "thesauros/math/integer-cast.hpp"
#include "thesauros/ranges/cartesian-product.hpp"
#include "thesauros/ranges/indices.hpp"
#include "thesauros/test/equality.hpp"
#include "thesauros/test/test.hpp"

namespace test = thes::test;

namespace {
using Pair = std::tuple<int, int>;
using Triple = std::tuple<int, int, int>;

// The factors of the products under test, whose values are disjoint so that the position of an
// element within the product is unambiguous.
constexpr std::array<int, 2> rows{0, 1};
constexpr std::array<int, 3> cols{10, 20, 30};
constexpr std::array<int, 2> layers{100, 200};

/** The 2×3 product, in the row-major order the view is required to produce. */
constexpr std::array<Pair, 6> pairs{Pair{0, 10}, Pair{0, 20}, Pair{0, 30},
                                    Pair{1, 10}, Pair{1, 20}, Pair{1, 30}};
/** The 2×3×2 product, in which the last factor varies fastest. */
constexpr std::array<Triple, 12> triples{
  Triple{0, 10, 100}, Triple{0, 10, 200}, Triple{0, 20, 100}, Triple{0, 20, 200},
  Triple{0, 30, 100}, Triple{0, 30, 200}, Triple{1, 10, 100}, Triple{1, 10, 200},
  Triple{1, 20, 100}, Triple{1, 20, 200}, Triple{1, 30, 100}, Triple{1, 30, 200}};

using Pairs = thes::ranges::CartesianProductView<std::views::all_t<const std::array<int, 2>&>,
                                                 std::views::all_t<const std::array<int, 3>&>>;
using Diff = std::ranges::range_difference_t<Pairs>;

/** The number of elements of the 2×3×2 product, as the difference type used for iterators. */
constexpr Diff triple_num = 12;

/** The 2×3×2 product, whose iterators have to carry across two factor boundaries. */
[[nodiscard]] auto make_triples() {
  return thes::views::cartesian_product(rows, cols, layers);
}

/** The expected element at the signed `offset`, which iterator arithmetic works with. */
[[nodiscard]] const Triple& triple_at(Diff offset) {
  return triples[*thes::safe_cast<std::size_t>(offset)];
}

//==================================================================================================
// Structure
//==================================================================================================

// `cartesian_product` produces exactly `CartesianProductView` over the views of its arguments.
static_assert(std::same_as<decltype(thes::views::cartesian_product(rows, cols)), Pairs>);

// Random-access sized factors make the product random-access, sized and common.
static_assert(std::ranges::random_access_range<Pairs>);
static_assert(std::ranges::sized_range<Pairs>);
static_assert(std::ranges::common_range<Pairs>);
static_assert(
  std::sized_sentinel_for<std::ranges::iterator_t<Pairs>, std::ranges::iterator_t<Pairs>>);

// The value type drops the references that the reference type retains.
static_assert(std::same_as<std::ranges::range_value_t<Pairs>, Pair>);
static_assert(
  std::same_as<std::ranges::range_reference_t<Pairs>, std::tuple<const int&, const int&>>);
static_assert(
  std::same_as<std::ranges::range_rvalue_reference_t<Pairs>, std::tuple<const int&&, const int&&>>);

//==================================================================================================
// Enumeration
//==================================================================================================

/** Checks that a two-factor product enumerates row-major, i.e. its last factor varies fastest. */
THES_TEST_CASE("a two-factor product enumerates row-major", "[ranges][cartesian-product]") {
  const Pairs view{rows, cols};

  THES_CHECK(test::range_eq(view, pairs));
  THES_CHECK(view.size() == 6);
  THES_CHECK(fmt::format("{}", view) == "[(0, 10), (0, 20), (0, 30), (1, 10), (1, 20), (1, 30)]");
}

/** Checks that the row-major order extends to more than two factors. */
THES_TEST_CASE("a three-factor product enumerates row-major", "[ranges][cartesian-product]") {
  const auto view = make_triples();

  THES_CHECK(test::range_eq(view, triples));
  THES_CHECK(view.size() == 12);
}

/** Checks the degenerate arities, where no factor at all yields the empty tuple exactly once. */
THES_TEST_CASE("products of zero and one factor", "[ranges][cartesian-product]") {
  const auto nullary = thes::views::cartesian_product();
  THES_CHECK(std::ranges::size(nullary) == 1);
  THES_CHECK(*std::ranges::begin(nullary) == std::tuple<>{});

  const auto unary = thes::views::cartesian_product(cols);
  THES_CHECK(std::ranges::size(unary) == 3);
  THES_CHECK(test::range_eq(unary, std::array{std::tuple{10}, std::tuple{20}, std::tuple{30}}));
}

/** Checks that an empty factor empties the whole product, wherever it appears. */
THES_TEST_CASE("an empty factor empties the product", "[ranges][cartesian-product]") {
  constexpr std::array<int, 0> nothing{};

  const auto trailing = thes::views::cartesian_product(rows, nothing);
  THES_CHECK(trailing.size() == 0);
  THES_CHECK(std::ranges::empty(trailing));
  THES_CHECK(trailing.begin() == trailing.end());
  THES_CHECK((trailing.begin() == std::default_sentinel));

  const auto leading = thes::views::cartesian_product(nothing, cols);
  THES_CHECK(leading.size() == 0);
  THES_CHECK(std::ranges::empty(leading));
  THES_CHECK(leading.begin() == leading.end());
  THES_CHECK((leading.begin() == std::default_sentinel));
}

/** Checks a product of `indices`, whose iterators yield values rather than references. */
THES_TEST_CASE("a product of index ranges yields values", "[ranges][cartesian-product]") {
  const auto view =
    thes::views::cartesian_product(thes::views::indices(2), thes::views::indices(3));

  THES_CHECK(
    test::range_eq(view, std::array{std::tuple{0, 0}, std::tuple{0, 1}, std::tuple{0, 2},
                                    std::tuple{1, 0}, std::tuple{1, 1}, std::tuple{1, 2}}));
  THES_CHECK(fmt::format("{}", view) == "[(0, 0), (0, 1), (0, 2), (1, 0), (1, 1), (1, 2)]");
}

//==================================================================================================
// Traversal
//==================================================================================================

/** Checks that incrementing to the end and decrementing back retraces the same elements. */
THES_TEST_CASE("increment and decrement retrace the product", "[ranges][cartesian-product]") {
  const auto view = make_triples();

  auto it = view.begin();
  for (const Triple& expected : triples) {
    const Triple value = *it;
    THES_CHECK(value == expected);
    ++it;
  }
  THES_CHECK(it == view.end());

  for (const Triple& expected : std::views::reverse(triples)) {
    --it;
    const Triple value = *it;
    THES_CHECK(value == expected);
  }
  THES_CHECK(it == view.begin());
}

/** Checks that the post-increment and post-decrement operators return the previous position. */
THES_TEST_CASE("post-increment and post-decrement return the old position",
               "[ranges][cartesian-product]") {
  const auto view = make_triples();

  auto it = view.begin();
  const auto before = it++;
  THES_CHECK(before == view.begin());
  THES_CHECK(it - before == 1);

  const auto after = it--;
  THES_CHECK(after - it == 1);
  THES_CHECK(it == view.begin());
}

/** Checks that the default sentinel marks exactly the end of the product. */
THES_TEST_CASE("the default sentinel marks the end", "[ranges][cartesian-product]") {
  const auto view = make_triples();

  auto it = view.begin();
  for (Diff i = 0; i < triple_num; ++i) {
    THES_CHECK((it != std::default_sentinel));
    ++it;
  }
  THES_CHECK((it == std::default_sentinel));
}

//==================================================================================================
// Random access
//==================================================================================================

/** Checks that advancing by an offset agrees with repeated increments, in both directions. */
THES_TEST_CASE("advancing by an offset matches repeated increments",
               "[ranges][cartesian-product]") {
  const auto view = make_triples();
  THES_REQUIRE(std::ranges::ssize(view) == triple_num);

  for (Diff offset = 0; offset <= triple_num; ++offset) {
    auto stepped = view.begin();
    for (Diff i = 0; i < offset; ++i) {
      ++stepped;
    }

    // Forwards from the beginning and backwards from the end reach the same position.
    THES_CHECK(view.begin() + offset == stepped);
    THES_CHECK(offset + view.begin() == stepped);
    THES_CHECK(view.end() - (triple_num - offset) == stepped);

    // The distances to both ends and to the sentinel are consistent with the position.
    THES_CHECK(stepped - view.begin() == offset);
    THES_CHECK(stepped - view.end() == offset - triple_num);
    THES_CHECK(std::default_sentinel - stepped == triple_num - offset);
    THES_CHECK(stepped - std::default_sentinel == offset - triple_num);
  }
}

/** Checks `+=` and `-=`, whose offsets have to be distributed over all factors. */
THES_TEST_CASE("compound assignment jumps across factor boundaries",
               "[ranges][cartesian-product]") {
  const auto view = make_triples();

  for (Diff offset = 0; offset < triple_num; ++offset) {
    auto forwards = view.begin();
    forwards += offset;
    const Triple from_begin = *forwards;
    THES_CHECK(from_begin == triple_at(offset));

    // The same element, reached with a negative offset from the end.
    auto backwards = view.end();
    backwards -= triple_num - offset;
    const Triple from_end = *backwards;
    THES_CHECK(from_end == triple_at(offset));
  }
}

/** Checks that subscripting an iterator and the view is equivalent to advancing. */
THES_TEST_CASE("subscripting matches advancing", "[ranges][cartesian-product]") {
  const auto view = make_triples();
  const auto begin = view.begin();

  for (Diff offset = 0; offset < triple_num; ++offset) {
    const Triple from_iterator = begin[offset];
    const Triple from_view = view[offset];
    THES_CHECK(from_iterator == triple_at(offset));
    THES_CHECK(from_view == triple_at(offset));
  }
}

/** Checks that iterators compare according to their position within the product. */
THES_TEST_CASE("iterators compare by position", "[ranges][cartesian-product]") {
  const auto view = make_triples();
  const auto begin = view.begin();

  THES_CHECK(begin == view.begin());
  THES_CHECK(begin <= view.begin());
  THES_CHECK(begin >= view.begin());
  THES_CHECK(begin != begin + 1);
  THES_CHECK(begin < begin + 1);
  THES_CHECK(begin + 1 > begin);
  // Positions that differ in an earlier factor still compare by their overall position.
  THES_CHECK(begin + 1 < begin + 2);
  THES_CHECK(begin + 5 < begin + 6);
  THES_CHECK(begin < view.end());
}

//==================================================================================================
// Modification
//==================================================================================================

/** Checks that assigning through the reference tuple writes back into the factors. */
THES_TEST_CASE("elements can be written through the product", "[ranges][cartesian-product]") {
  std::array<int, 2> first{1, 2};
  std::array<int, 2> second{3, 4};
  const auto view = thes::views::cartesian_product(first, second);

  std::get<0>(*view.begin()) = 10;
  std::get<1>(*view.begin()) = 30;

  THES_CHECK(test::range_eq(first, std::array{10, 2}));
  THES_CHECK(test::range_eq(second, std::array{30, 4}));
}

/** Checks that `iter_swap` swaps the referenced elements of every factor. */
THES_TEST_CASE("iter_swap swaps element-wise", "[ranges][cartesian-product]") {
  std::array<int, 2> first{1, 2};
  std::array<int, 2> second{3, 4};
  const auto view = thes::views::cartesian_product(first, second);

  // The first and the last element of the product refer to different elements of both factors.
  std::ranges::iter_swap(view.begin(), view.begin() + 3);

  THES_CHECK(test::range_eq(first, std::array{2, 1}));
  THES_CHECK(test::range_eq(second, std::array{4, 3}));
}

//==================================================================================================
// Interoperability
//==================================================================================================

/** Checks that the product works with the standard range algorithms. */
THES_TEST_CASE("the product works with range algorithms", "[ranges][cartesian-product]") {
  const auto view = make_triples();

  THES_CHECK(std::ranges::distance(view) == triple_num);
  THES_CHECK(std::ranges::count(view, Triple{0, 20, 200}) == 1);
  THES_CHECK(std::ranges::find(view, Triple{1, 10, 100}) - view.begin() == 6);

  const auto sums = view | std::views::transform([](const auto& element) {
                      return std::get<0>(element) + std::get<1>(element) + std::get<2>(element);
                    });
  THES_CHECK(std::ranges::fold_left(sums, 0, std::plus{}) == 2046);
}

/** Checks that a `const` product iterates like a mutable one. */
THES_TEST_CASE("a const product agrees with a mutable one", "[ranges][cartesian-product]") {
  auto mutable_view = thes::views::cartesian_product(rows, cols);
  const auto& const_view = mutable_view;

  THES_CHECK(test::range_eq(mutable_view, const_view));
  THES_CHECK(mutable_view.size() == const_view.size());
  THES_CHECK(mutable_view.begin() == const_view.begin());
  THES_CHECK(mutable_view.end() == const_view.end());
}
} // namespace

THES_TEST_MAIN()
