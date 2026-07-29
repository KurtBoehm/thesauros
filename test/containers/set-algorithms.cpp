// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

#include "thesauros/containers/set-algorithms.hpp"
#include "thesauros/test/equality.hpp"
#include "thesauros/test/test.hpp"

namespace test = thes::test;

namespace {
using Vec = std::vector<int>;

/** A key/value pair ordered by its key alone, to exercise the comparator/equality parameters. */
struct Entry {
  int key{};
  std::string tag{};

  bool operator==(const Entry&) const = default;
};

struct EntryLess {
  bool operator()(const Entry& e1, const Entry& e2) const {
    return e1.key < e2.key;
  }
};
struct EntryEqual {
  bool operator()(const Entry& e1, const Entry& e2) const {
    return e1.key == e2.key;
  }
};

//==================================================================================================
// erase_if
//==================================================================================================

/** Checks that `erase_if` removes exactly the matching elements and keeps the rest in order. */
THES_TEST_CASE("erase_if removes matching elements", "[containers][set-algorithms]") {
  Vec values{1, 2, 3, 4, 5, 6};
  thes::erase_if(values, [](int v) { return v % 2 == 0; });
  THES_CHECK(test::range_eq(values, Vec{1, 3, 5}));

  thes::erase_if(values, [](int /*v*/) { return false; });
  THES_CHECK(test::range_eq(values, Vec{1, 3, 5}));

  thes::erase_if(values, [](int /*v*/) { return true; });
  THES_CHECK(values.empty());

  Vec empty{};
  thes::erase_if(empty, [](int /*v*/) { return true; });
  THES_CHECK(empty.empty());
}

//==================================================================================================
// set_union
//==================================================================================================

/** Checks that `set_union` yields the sorted union without duplicating shared elements. */
THES_TEST_CASE("set_union merges sorted ranges", "[containers][set-algorithms]") {
  Vec values{1, 3, 5};
  thes::set_union(values, Vec{2, 3, 6});
  THES_CHECK(test::range_eq(values, Vec{1, 2, 3, 5, 6}));
}

/** Checks the boundary cases in which one of the two ranges is empty or fully disjoint. */
THES_TEST_CASE("set_union handles empty and disjoint ranges", "[containers][set-algorithms]") {
  {
    Vec values{};
    thes::set_union(values, Vec{1, 2, 3});
    THES_CHECK(test::range_eq(values, Vec{1, 2, 3}));
  }
  {
    Vec values{1, 2, 3};
    thes::set_union(values, Vec{});
    THES_CHECK(test::range_eq(values, Vec{1, 2, 3}));
  }
  {
    // Every element of the second range sorts before the single element of the first.
    Vec values{5};
    thes::set_union(values, Vec{1, 2});
    THES_CHECK(test::range_eq(values, Vec{1, 2, 5}));
  }
  {
    // Every element of the second range sorts after the whole first range.
    Vec values{1, 2};
    thes::set_union(values, Vec{5, 6});
    THES_CHECK(test::range_eq(values, Vec{1, 2, 5, 6}));
  }
}

/** Checks that elements already present are not appended a second time. */
THES_TEST_CASE("set_union does not duplicate shared elements", "[containers][set-algorithms]") {
  Vec values{1, 2, 3};
  thes::set_union(values, Vec{1, 2, 3});
  THES_CHECK(test::range_eq(values, Vec{1, 2, 3}));

  thes::set_union(values, Vec{2, 2});
  THES_CHECK(test::range_eq(values, Vec{1, 2, 3}));
}

/** Checks that a custom comparator and equality relation decide which elements are shared. */
THES_TEST_CASE("set_union honours custom comparators", "[containers][set-algorithms]") {
  std::vector<Entry> values{Entry{1, "a"}, Entry{3, "c"}};
  const std::vector<Entry> other{Entry{2, "B"}, Entry{3, "C"}};

  thes::set_union(values, other, EntryLess{}, EntryEqual{});
  const std::vector<Entry> expected{Entry{1, "a"}, Entry{2, "B"},
                                    // The existing entry wins, so its payload is left untouched.
                                    Entry{3, "c"}};
  THES_CHECK(test::range_eq(values, expected));
}

//==================================================================================================
// set_difference
//==================================================================================================

/** Checks that `set_difference` removes the shared elements and preserves the order of the rest. */
THES_TEST_CASE("set_difference removes shared elements", "[containers][set-algorithms]") {
  Vec values{1, 2, 3, 4};
  thes::set_difference(values, Vec{2, 4});
  THES_CHECK(test::range_eq(values, Vec{1, 3}));
}

/** Checks the boundary cases of `set_difference`. */
THES_TEST_CASE("set_difference handles empty and identical ranges",
               "[containers][set-algorithms]") {
  {
    Vec values{1, 2, 3};
    thes::set_difference(values, Vec{});
    THES_CHECK(test::range_eq(values, Vec{1, 2, 3}));
  }
  {
    Vec values{1, 2, 3};
    thes::set_difference(values, Vec{1, 2, 3});
    THES_CHECK(values.empty());
  }
  {
    Vec values{};
    thes::set_difference(values, Vec{1, 2, 3});
    THES_CHECK(values.empty());
  }
  {
    // Nothing is shared, so the range survives unchanged.
    Vec values{1, 3, 5};
    thes::set_difference(values, Vec{2, 4, 6});
    THES_CHECK(test::range_eq(values, Vec{1, 3, 5}));
  }
}

/** Checks `set_difference` against `std::set_difference` over a range of subsets. */
THES_TEST_CASE("set_difference matches the standard algorithm", "[containers][set-algorithms]") {
  static constexpr int size = 8;

  for (unsigned bits = 0; bits < (1U << unsigned{size}); ++bits) {
    Vec values{};
    Vec other{};
    for (int i = 0; i < size; ++i) {
      values.push_back(i);
      if ((bits & (1U << unsigned(i))) != 0) {
        other.push_back(i);
      }
    }

    Vec expected{};
    std::ranges::set_difference(values, other, std::back_inserter(expected));

    thes::set_difference(values, other);
    THES_CHECK(test::range_eq(values, expected));
  }
}

//==================================================================================================
// find_sorted and exists_sorted
//==================================================================================================

/** Checks that `find_sorted` returns the matching iterator, or `end` when the value is absent. */
THES_TEST_CASE("find_sorted locates values in a sorted range", "[containers][set-algorithms]") {
  const Vec values{1, 3, 5, 7};

  THES_CHECK(thes::find_sorted(values.begin(), values.end(), 1) == values.begin());
  THES_CHECK(thes::find_sorted(values.begin(), values.end(), 7) == values.end() - 1);
  THES_CHECK(thes::find_sorted(values.begin(), values.end(), 5) - values.begin() == 2);

  THES_CHECK(thes::find_sorted(values.begin(), values.end(), 0) == values.end());
  THES_CHECK(thes::find_sorted(values.begin(), values.end(), 4) == values.end());
  THES_CHECK(thes::find_sorted(values.begin(), values.end(), 8) == values.end());

  const Vec empty{};
  THES_CHECK(thes::find_sorted(empty.begin(), empty.end(), 1) == empty.end());
}

/** Checks that `exists_sorted` agrees with `find_sorted`, including with a custom comparator. */
THES_TEST_CASE("exists_sorted reports membership", "[containers][set-algorithms]") {
  const Vec values{1, 3, 5, 7};
  for (int value = 0; value <= 8; ++value) {
    const bool expected = std::ranges::find(values, value) != values.end();
    THES_CHECK(thes::exists_sorted(values.begin(), values.end(), value) == expected);
  }

  const std::vector<Entry> entries{Entry{.key = 1, .tag = "a"}, Entry{.key = 3, .tag = "c"}};
  const Entry present{.key = 3, .tag = "other"};
  const Entry absent{.key = 2, .tag = "b"};
  THES_CHECK(
    thes::exists_sorted(entries.begin(), entries.end(), present, EntryLess{}, EntryEqual{}));
  THES_CHECK(
    !thes::exists_sorted(entries.begin(), entries.end(), absent, EntryLess{}, EntryEqual{}));
}
} // namespace

THES_TEST_MAIN()
