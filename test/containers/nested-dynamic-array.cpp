// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <span>
#include <vector>

#include "thesauros/containers/nested-dynamic-array.hpp"
#include "thesauros/test/equality.hpp"
#include "thesauros/test/test.hpp"

namespace test = thes::test;

namespace {
using Nested = thes::NestedDynamicArray<int, std::size_t>;
using Groups = std::vector<std::vector<int>>;

static_assert(std::ranges::random_access_range<Nested>);
static_assert(std::ranges::random_access_range<const Nested>);

/** Builds a `Nested` from `groups` using `FlatBuilder`, which fills all groups in order. */
[[nodiscard]] Nested build_flat(const Groups& groups) {
  std::size_t element_num = 0;
  for (const auto& group : groups) {
    element_num += group.size();
  }

  Nested::FlatBuilder builder{};
  builder.initialize(groups.size(), element_num);
  for (const auto& group : groups) {
    for (int value : group) {
      builder.emplace(value);
    }
    builder.advance_group();
  }
  return builder.build();
}

/** Builds a `Nested` from `groups` using `NestedBuilder`, whose parts can be filled separately. */
[[nodiscard]] Nested build_nested(const Groups& groups) {
  std::size_t element_num = 0;
  for (const auto& group : groups) {
    element_num += group.size();
  }

  Nested::NestedBuilder builder{};
  builder.initialize(groups.size(), element_num);

  std::size_t group_index = 0;
  std::size_t value_index = 0;
  for (const auto& group : groups) {
    auto part = builder.part_builder(group_index, value_index);
    for (int value : group) {
      part.emplace(value);
    }
    part.advance_group();
    ++group_index;
    value_index += group.size();
  }
  return builder.build();
}

/** Checks that `nested` has exactly the groups of `groups`, both by index and by iteration. */
[[nodiscard]] bool matches(const Nested& nested, const Groups& groups) {
  if (nested.size() != groups.size() || nested.group_num() != groups.size()) {
    return false;
  }

  std::size_t element_num = 0;
  for (std::size_t i = 0; i < groups.size(); ++i) {
    if (!test::range_eq(nested[i], groups[i])) {
      return false;
    }
    element_num += groups[i].size();
  }
  if (nested.element_num() != element_num || nested.flat_size() != element_num) {
    return false;
  }

  std::size_t index = 0;
  for (std::span<const int> group : nested) {
    if (index >= groups.size() || !test::range_eq(group, groups[index])) {
      return false;
    }
    ++index;
  }
  return index == groups.size();
}

const Groups sample{{1, 2, 3}, {}, {4}, {5, 6}};

//==================================================================================================
// Builders
//==================================================================================================

/** Checks that `FlatBuilder` reproduces the groups it was fed, including an empty one. */
THES_TEST_CASE("FlatBuilder reproduces its input", "[containers][nested-dynamic-array]") {
  const Nested nested = build_flat(sample);
  THES_CHECK(matches(nested, sample));
}

/** Checks that `NestedBuilder` and `FlatBuilder` produce the same container. */
THES_TEST_CASE("NestedBuilder matches FlatBuilder", "[containers][nested-dynamic-array]") {
  const Nested flat = build_flat(sample);
  const Nested nested = build_nested(sample);

  THES_REQUIRE(flat.size() == nested.size());
  THES_CHECK(test::range_eq(flat.offsets(), nested.offsets()));
  THES_CHECK(test::range_eq(flat.values(), nested.values()));
  THES_CHECK(matches(nested, sample));
}

/** Checks the offsets themselves, which are the container’s actual representation. */
THES_TEST_CASE("the offsets delimit the groups", "[containers][nested-dynamic-array]") {
  const Nested nested = build_flat(sample);

  THES_CHECK(test::range_eq(nested.offsets(), std::vector<std::size_t>{0, 3, 3, 4, 6}));
  THES_CHECK(test::range_eq(nested.values(), std::vector<int>{1, 2, 3, 4, 5, 6}));

  THES_CHECK(test::range_eq(nested.offsets_of(0), std::vector<std::size_t>{0, 1, 2}));
  THES_CHECK(test::range_eq(nested.offsets_of(1), std::vector<std::size_t>{}));
  THES_CHECK(test::range_eq(nested.offsets_of(2), std::vector<std::size_t>{3}));
  THES_CHECK(test::range_eq(nested.offsets_of(3), std::vector<std::size_t>{4, 5}));
}

/** Checks a container with no groups at all, whose single offset is zero. */
THES_TEST_CASE("an empty container has one offset", "[containers][nested-dynamic-array]") {
  const Nested nested = build_flat(Groups{});

  THES_CHECK(nested.empty());
  THES_CHECK(nested.size() == 0);
  THES_CHECK(nested.group_num() == 0);
  THES_CHECK(nested.element_num() == 0);
  THES_CHECK(nested.offsets().size() == 1);
  THES_CHECK(nested.offsets()[0] == 0);
  THES_CHECK(nested.begin() == nested.end());
}

/** Checks a container whose groups are all empty, so that every offset is zero. */
THES_TEST_CASE("groups may all be empty", "[containers][nested-dynamic-array]") {
  const Groups groups{{}, {}, {}};
  const Nested nested = build_flat(groups);

  THES_CHECK(!nested.empty());
  THES_CHECK(nested.size() == 3);
  THES_CHECK(nested.element_num() == 0);
  THES_CHECK(test::range_eq(nested.offsets(), std::vector<std::size_t>{0, 0, 0, 0}));
  THES_CHECK(matches(nested, groups));
}

//==================================================================================================
// Element access
//==================================================================================================

/** Checks `front`, `back` and mutation through the non-const `operator[]`. */
THES_TEST_CASE("front, back and mutable access", "[containers][nested-dynamic-array]") {
  Nested nested = build_flat(sample);

  THES_CHECK(test::range_eq(nested.front(), sample.front()));
  THES_CHECK(test::range_eq(nested.back(), sample.back()));

  const Nested& cnested = nested;
  THES_CHECK(test::range_eq(cnested.front(), sample.front()));
  THES_CHECK(test::range_eq(cnested.back(), sample.back()));

  // The spans alias the container’s storage, so writing through one is visible afterwards.
  nested[0][1] = 20;
  THES_CHECK(cnested[0][1] == 20);
  THES_CHECK(cnested.values()[1] == 20);

  std::ranges::fill(nested.back(), 7);
  THES_CHECK(test::range_eq(cnested.back(), std::vector<int>{7, 7}));
}

//==================================================================================================
// Iteration
//==================================================================================================

/** Checks that iteration visits each group once, in order, for both iterator constnesses. */
THES_TEST_CASE("iteration visits every group", "[containers][nested-dynamic-array]") {
  Nested nested = build_flat(sample);
  const Nested& cnested = nested;

  THES_CHECK(std::distance(nested.begin(), nested.end()) == 4);
  THES_CHECK(std::distance(cnested.begin(), cnested.end()) == 4);

  auto it = cnested.begin();
  THES_CHECK(test::range_eq(*it, sample[0]));
  ++it;
  THES_CHECK((*it).empty());
  ++it;
  THES_CHECK(test::range_eq(*it, sample[2]));
  ++it;
  THES_CHECK(test::range_eq(*it, sample[3]));
  ++it;
  THES_CHECK(it == cnested.end());

  // Writing through a mutable iterator reaches the container’s storage.
  (*nested.begin())[0] = 42;
  THES_CHECK(cnested[0][0] == 42);
}

/** Checks the sizes of the groups seen through a `std::views::transform` over the container. */
THES_TEST_CASE("the container works as a range", "[containers][nested-dynamic-array]") {
  const Nested nested = build_flat(sample);
  const auto sizes = nested | std::views::transform([](auto group) { return group.size(); });

  THES_CHECK(test::range_eq(sizes, std::vector<std::size_t>{3, 0, 1, 2}));
}
} // namespace

THES_TEST_MAIN()
