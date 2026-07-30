// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <vector>

#include "thesauros/containers/bitset/static.hpp"
#include "thesauros/static-ranges/definitions/get-at.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/piping.hpp" // IWYU pragma: keep
#include "thesauros/static-ranges/sinks/to-array.hpp"
#include "thesauros/static-ranges/sinks/to-container.hpp"
#include "thesauros/static-ranges/sinks/to-static-bitset.hpp"
#include "thesauros/static-ranges/views/elements.hpp"
#include "thesauros/static-ranges/views/iota.hpp"
#include "thesauros/static-ranges/views/reversed.hpp"
#include "thesauros/static-ranges/views/transform.hpp"
#include "thesauros/test/equality.hpp"
#include "thesauros/test/test.hpp"
#include "thesauros/types/primitives.hpp"

namespace star = thes::star;
namespace test = thes::test;

namespace {
//==================================================================================================
// reversed
//==================================================================================================

static_assert((std::array{1, 2, 3} | star::reversed | star::to_array) == std::array{3, 2, 1});
// Reversing twice is the identity.
static_assert((std::array{1, 2, 3} | star::reversed | star::reversed | star::to_array) ==
              std::array{1, 2, 3});
// A one-element range, where reversing changes nothing.
static_assert((std::array{7} | star::reversed | star::to_array) == std::array{7});
static_assert((star::iota<0, 6> | star::reversed | star::to_array) ==
              std::array<std::size_t, 6>{5, 4, 3, 2, 1, 0});

/** Checks that `reversed` walks a heterogeneous tuple back to front. */
THES_TEST_CASE("reversed handles heterogeneous ranges", "[static-ranges][reversed]") {
  const std::tuple values{1, 2.5, std::string{"three"}};
  const auto flipped = values | star::reversed;

  static_assert(star::size<decltype(flipped)> == 3);
  THES_CHECK(star::get_at<0>(flipped) == "three");
  THES_CHECK(star::get_at<1>(flipped) == 2.5);
  THES_CHECK(star::get_at<2>(flipped) == 1);
}

/** Checks that `reversed` composes with `transform`, in either order. */
THES_TEST_CASE("reversed composes with transform", "[static-ranges][reversed]") {
  static constexpr auto doubled = star::transform([](int v) { return 2 * v; });

  const auto first = std::array{1, 2, 3} | star::reversed | doubled | star::to_array;
  const auto second = std::array{1, 2, 3} | doubled | star::reversed | star::to_array;

  THES_CHECK(test::range_eq(first, std::array{6, 4, 2}));
  THES_CHECK(test::range_eq(second, std::array{6, 4, 2}));
}

//==================================================================================================
// elements
//==================================================================================================

/** Checks that `elements<I>` projects the `I`-th component out of each element. */
THES_TEST_CASE("elements projects a component", "[static-ranges][elements]") {
  const std::array pairs{std::tuple{1, 'a'}, std::tuple{2, 'b'}, std::tuple{3, 'c'}};

  const auto firsts = pairs | star::elements<0> | star::to_array;
  const auto seconds = pairs | star::elements<1> | star::to_array;

  THES_CHECK(test::range_eq(firsts, std::array{1, 2, 3}));
  THES_CHECK(test::range_eq(seconds, std::array{'a', 'b', 'c'}));
}

/** Checks that `elements` composes with the other views. */
THES_TEST_CASE("elements composes with reversed", "[static-ranges][elements]") {
  const std::array pairs{std::tuple{1, 10}, std::tuple{2, 20}, std::tuple{3, 30}};

  const auto values = pairs | star::elements<1> | star::reversed | star::to_array;
  THES_CHECK(test::range_eq(values, std::array{30, 20, 10}));
}

/** Checks that `elements` works over a tuple of tuples with differing component types. */
THES_TEST_CASE("elements handles heterogeneous components", "[static-ranges][elements]") {
  const std::tuple rows{std::tuple{1, 'x'}, std::tuple{2.5, 'y'}};

  const auto keys = rows | star::elements<0>;
  THES_CHECK(star::get_at<0>(keys) == 1);
  THES_CHECK(star::get_at<1>(keys) == 2.5);

  const auto tags = rows | star::elements<1> | star::to_array;
  THES_CHECK(test::range_eq(tags, std::array{'x', 'y'}));
}

//==================================================================================================
// to_container
//==================================================================================================

/**
 * Checks that `to_container` brace-initializes the target from the elements. Because that is a
 * braced initializer list, the element types have to be convertible without narrowing.
 */
THES_TEST_CASE("to_container builds arbitrary containers", "[static-ranges][to-container]") {
  const auto vec = std::array{1, 2, 3, 4} | star::to_container<std::vector<int>>;
  THES_CHECK(test::range_eq(vec, std::array{1, 2, 3, 4}));

  const auto indices = star::iota<1, 5> | star::to_container<std::vector<std::size_t>>;
  THES_CHECK(test::range_eq(indices, std::array<std::size_t, 4>{1, 2, 3, 4}));

  const auto text = std::array{'a', 'b', 'c'} | star::to_container<std::string>;
  THES_CHECK(text == "abc");
}

/** Checks that a heterogeneous range lands in a container whose type all elements widen to. */
THES_TEST_CASE("to_container accepts heterogeneous input", "[static-ranges][to-container]") {
  const std::tuple values{thes::u8{1}, thes::u16{2}, thes::u32{3}};
  const auto vec = values | star::to_container<std::vector<thes::u32>>;
  THES_CHECK(test::range_eq(vec, std::array<thes::u32, 3>{1, 2, 3}));
}

//==================================================================================================
// to_static_bitset
//==================================================================================================

/** Checks that a range of `bool` becomes a `StaticBitset` with the same bits set. */
THES_TEST_CASE("to_static_bitset packs a boolean range", "[static-ranges][to-static-bitset]") {
  const auto bits = std::array{true, false, true, true} | star::to_static_bitset;
  static_assert(star::size<decltype(std::array{true, false, true, true})> == 4);

  THES_CHECK(bits[0]);
  THES_CHECK(!bits[1]);
  THES_CHECK(bits[2]);
  THES_CHECK(bits[3]);
}

/** Checks the all-set and none-set extremes, and a bitset built from a transform. */
THES_TEST_CASE("to_static_bitset handles uniform ranges", "[static-ranges][to-static-bitset]") {
  const auto all = std::array{true, true, true} | star::to_static_bitset;
  const auto none = std::array{false, false, false} | star::to_static_bitset;

  for (std::size_t i = 0; i < 3; ++i) {
    THES_CHECK(all[i]);
    THES_CHECK(!none[i]);
  }

  // Deriving the bits from a computation is the usual way this sink is reached.
  const auto even = star::iota<0, 6> | star::transform([](auto tag) { return tag % 2 == 0; }) |
                    star::to_static_bitset;
  for (std::size_t i = 0; i < 6; ++i) {
    THES_CHECK(even[i] == (i % 2 == 0));
  }
}
} // namespace

THES_TEST_MAIN()
