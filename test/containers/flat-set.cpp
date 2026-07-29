// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <functional>
#include <initializer_list>
#include <string>
#include <vector>

#include "thesauros/containers/flat-set.hpp"
#include "thesauros/test/equality.hpp"
#include "thesauros/test/test.hpp"

namespace test = thes::test;

namespace {
using Set = thes::FlatSet<int>;
using Vec = std::vector<int>;

/** Builds a set from `values`, inserting them in the given order. */
[[nodiscard]] Set make_set(std::initializer_list<int> values) {
  Set set{};
  for (int value : values) {
    set.insert(value);
  }
  return set;
}

//==================================================================================================
// Ordering and iteration
//==================================================================================================

/** Checks that the values are kept sorted and deduplicated. */
THES_TEST_CASE("values stay sorted and unique", "[containers][flat-set]") {
  const Set set = make_set({5, 1, 3, 1, 5, 2});

  THES_CHECK(test::range_eq(set, Vec{1, 2, 3, 5}));
  THES_CHECK(set.size() == 4);
  THES_CHECK(!set.empty());
  THES_CHECK(set.end() - set.begin() == 4);
  THES_CHECK(set.cend() - set.cbegin() == 4);
}

/** Checks the empty state. */
THES_TEST_CASE("a fresh set is empty", "[containers][flat-set]") {
  const Set set{};

  THES_CHECK(set.empty());
  THES_CHECK(set.size() == 0);
  THES_CHECK(set.begin() == set.end());
  THES_CHECK(!set.contains(0));
  THES_CHECK(set.find(0) == set.end());
}

/** Checks that `front` and `pop_front` address the smallest value. */
THES_TEST_CASE("front and pop_front address the smallest value", "[containers][flat-set]") {
  Set set = make_set({3, 1, 2});

  THES_CHECK(set.front() == 1);
  set.pop_front();
  THES_CHECK(set.front() == 2);
  THES_CHECK(test::range_eq(set, Vec{2, 3}));

  set.pop_front();
  set.pop_front();
  THES_CHECK(set.empty());
}

//==================================================================================================
// Lookup
//==================================================================================================

/** Checks `contains`, `find` and `lower_bound` for present and absent values. */
THES_TEST_CASE("lookup finds present values only", "[containers][flat-set]") {
  const Set set = make_set({1, 3, 5});

  THES_CHECK(set.contains(1));
  THES_CHECK(set.contains(3));
  THES_CHECK(set.contains(5));
  THES_CHECK(!set.contains(0));
  THES_CHECK(!set.contains(4));
  THES_CHECK(!set.contains(6));

  THES_REQUIRE(set.find(3) != set.end());
  THES_CHECK(*set.find(3) == 3);
  THES_CHECK(set.find(4) == set.end());

  // `lower_bound` yields the first value not ordered before the argument.
  THES_CHECK(*set.lower_bound(0) == 1);
  THES_CHECK(*set.lower_bound(3) == 3);
  THES_CHECK(*set.lower_bound(4) == 5);
  THES_CHECK(set.lower_bound(6) == set.end());
}

//==================================================================================================
// Insertion and removal
//==================================================================================================

/** Checks that inserting an existing value leaves the set untouched. */
THES_TEST_CASE("insert is idempotent", "[containers][flat-set]") {
  Set set{};

  set.insert(2);
  set.insert(1);
  THES_CHECK(test::range_eq(set, Vec{1, 2}));

  set.insert(2);
  set.insert(1);
  THES_CHECK(test::range_eq(set, Vec{1, 2}));
  THES_CHECK(set.size() == 2);
}

/** Checks that `erase` reports whether it removed anything. */
THES_TEST_CASE("erase removes a single value", "[containers][flat-set]") {
  Set set = make_set({1, 2, 3});

  THES_CHECK(set.erase(2));
  THES_CHECK(!set.erase(2));
  THES_CHECK(!set.erase(9));
  THES_CHECK(test::range_eq(set, Vec{1, 3}));
}

/** Checks that `erase_if` removes every matching value at once. */
THES_TEST_CASE("erase_if removes matching values", "[containers][flat-set]") {
  Set set = make_set({1, 2, 3, 4, 5});

  set.erase_if([](int value) { return value % 2 == 0; });
  THES_CHECK(test::range_eq(set, Vec{1, 3, 5}));

  set.erase_if([](int /*value*/) { return false; });
  THES_CHECK(set.size() == 3);

  set.erase_if([](int /*value*/) { return true; });
  THES_CHECK(set.empty());
}

/** Checks that `clear` empties the set without breaking later use. */
THES_TEST_CASE("clear empties the set", "[containers][flat-set]") {
  Set set = make_set({1, 2});

  set.clear();
  THES_CHECK(set.empty());
  THES_CHECK(set.begin() == set.end());

  set.insert(7);
  THES_CHECK(test::range_eq(set, Vec{7}));
}

//==================================================================================================
// Set operations
//==================================================================================================

/** Checks that `set_union` merges in another sorted range without duplicating. */
THES_TEST_CASE("set_union merges another range", "[containers][flat-set]") {
  Set set = make_set({1, 3, 5});

  set.set_union(Vec{2, 3, 6});
  THES_CHECK(test::range_eq(set, Vec{1, 2, 3, 5, 6}));

  // Merging a subset changes nothing.
  set.set_union(Vec{1, 5});
  THES_CHECK(test::range_eq(set, Vec{1, 2, 3, 5, 6}));

  // Merging an empty range changes nothing either.
  set.set_union(Vec{});
  THES_CHECK(test::range_eq(set, Vec{1, 2, 3, 5, 6}));
}

/** Checks that `set_difference` removes the values shared with another sorted range. */
THES_TEST_CASE("set_difference removes shared values", "[containers][flat-set]") {
  Set set = make_set({1, 2, 3, 4});

  set.set_difference(Vec{2, 4});
  THES_CHECK(test::range_eq(set, Vec{1, 3}));

  // Subtracting values that are not present changes nothing.
  set.set_difference(Vec{7, 8});
  THES_CHECK(test::range_eq(set, Vec{1, 3}));

  set.set_difference(Vec{1, 3});
  THES_CHECK(set.empty());
}

//==================================================================================================
// Custom comparators
//==================================================================================================

/** Checks that a reversed comparator flips the stored order while lookup keeps working. */
THES_TEST_CASE("a custom comparator reverses the order", "[containers][flat-set]") {
  thes::FlatSet<int, std::greater<>> set{};
  for (int value : {1, 3, 2}) {
    set.insert(value);
  }

  THES_CHECK(test::range_eq(set, Vec{3, 2, 1}));
  THES_CHECK(set.front() == 3);
  THES_CHECK(set.contains(2));
  THES_CHECK(!set.contains(4));
  THES_CHECK(*set.find(1) == 1);
}

/** Checks that a non-trivial value type works, exercising copies rather than raw bytes. */
THES_TEST_CASE("a string set behaves the same", "[containers][flat-set]") {
  thes::FlatSet<std::string> set{};
  for (const char* value : {"pear", "apple", "pear", "fig"}) {
    set.insert(std::string{value});
  }

  THES_CHECK(test::range_eq(set, std::vector<std::string>{"apple", "fig", "pear"}));
  THES_CHECK(set.contains(std::string{"fig"}));
  THES_CHECK(!set.contains(std::string{"plum"}));
  THES_CHECK(set.erase(std::string{"fig"}));
  THES_CHECK(test::range_eq(set, std::vector<std::string>{"apple", "pear"}));
}
} // namespace

THES_TEST_MAIN()
