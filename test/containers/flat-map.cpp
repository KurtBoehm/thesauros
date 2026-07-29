// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "thesauros/containers/flat-map.hpp"
#include "thesauros/test/equality.hpp"
#include "thesauros/test/test.hpp"

namespace test = thes::test;

namespace {
using Map = thes::FlatMap<int, int>;
using Entries = std::vector<std::pair<int, int>>;

/** Builds a map from `entries`, inserting them in the given order. */
[[nodiscard]] Map make_map(const Entries& entries) {
  Map map{};
  for (const auto& [key, value] : entries) {
    map.insert(key, value);
  }
  return map;
}

//==================================================================================================
// Ordering and iteration
//==================================================================================================

/** Checks that the entries are kept sorted by key, whatever the insertion order. */
THES_TEST_CASE("entries stay sorted by key", "[containers][flat-map]") {
  const Map map = make_map({{5, 50}, {1, 10}, {3, 30}, {2, 20}});
  THES_CHECK(test::range_eq(map, Entries{{1, 10}, {2, 20}, {3, 30}, {5, 50}}));
  THES_CHECK(map.size() == 4);
  THES_CHECK(!map.empty());
}

/** Checks the empty state and that `front`/`pop_front` work off the smallest key. */
THES_TEST_CASE("front and pop_front address the smallest key", "[containers][flat-map]") {
  Map map = make_map({{3, 30}, {1, 10}, {2, 20}});

  THES_CHECK(map.front().first == 1);
  THES_CHECK(map.front().second == 10);

  map.front().second = 11;
  THES_CHECK(map.at(1) == 11);

  map.pop_front();
  THES_CHECK(map.size() == 2);
  THES_CHECK(map.front().first == 2);

  map.pop_front();
  map.pop_front();
  THES_CHECK(map.empty());
}

/** Checks that the const and non-const iterators agree. */
THES_TEST_CASE("const and mutable iteration agree", "[containers][flat-map]") {
  Map map = make_map({{2, 20}, {1, 10}});
  const Map& cmap = map;

  THES_CHECK(map.end() - map.begin() == 2);
  THES_CHECK(cmap.cend() - cmap.cbegin() == 2);
  THES_CHECK(test::range_eq(cmap, Entries{{1, 10}, {2, 20}}));

  // Writing through the mutable iterator changes the mapped value.
  map.begin()->second = 11;
  THES_CHECK(cmap.begin()->second == 11);
}

//==================================================================================================
// Lookup
//==================================================================================================

/** Checks `contains`, `find` and `lower_bound` for present and absent keys. */
THES_TEST_CASE("lookup finds present keys only", "[containers][flat-map]") {
  Map map = make_map({{1, 10}, {3, 30}, {5, 50}});
  const Map& cmap = map;

  THES_CHECK(map.contains(1));
  THES_CHECK(map.contains(5));
  THES_CHECK(!map.contains(0));
  THES_CHECK(!map.contains(4));
  THES_CHECK(!map.contains(6));

  THES_REQUIRE(map.find(3) != map.end());
  THES_CHECK(map.find(3)->second == 30);
  THES_CHECK(map.find(4) == map.end());
  THES_CHECK(cmap.find(3)->second == 30);
  THES_CHECK(cmap.find(4) == cmap.end());

  // `lower_bound` yields the first entry not ordered before the key, even when it is absent.
  THES_CHECK(map.lower_bound(0)->first == 1);
  THES_CHECK(map.lower_bound(3)->first == 3);
  THES_CHECK(map.lower_bound(4)->first == 5);
  THES_CHECK(map.lower_bound(6) == map.end());
  THES_CHECK(cmap.lower_bound(4)->first == 5);
}

/** Checks that `at` reaches the mapped value for reading and writing. */
THES_TEST_CASE("at reaches the mapped value", "[containers][flat-map]") {
  Map map = make_map({{1, 10}, {2, 20}});
  const Map& cmap = map;

  THES_CHECK(map.at(1) == 10);
  THES_CHECK(cmap.at(2) == 20);

  map.at(1) = 11;
  THES_CHECK(cmap.at(1) == 11);
}

//==================================================================================================
// Insertion
//==================================================================================================

/** Checks that `insert` reports whether it added anything and never overwrites. */
THES_TEST_CASE("insert does not overwrite", "[containers][flat-map]") {
  Map map{};

  THES_CHECK(map.insert(2, 20));
  THES_CHECK(map.insert(1, 10));
  THES_CHECK(!map.insert(2, 99));

  THES_CHECK(map.at(2) == 20);
  THES_CHECK(test::range_eq(map, Entries{{1, 10}, {2, 20}}));
}

/** Checks that `get_or_insert` returns a usable reference in both branches. */
THES_TEST_CASE("get_or_insert returns a mapped reference", "[containers][flat-map]") {
  Map map = make_map({{1, 10}});

  // The key is present, so the existing value is returned and the argument is discarded.
  int& existing = map.get_or_insert(1, 99);
  THES_CHECK(existing == 10);
  existing = 11;
  THES_CHECK(map.at(1) == 11);

  // The key is absent, so it is inserted with the supplied value.
  int& fresh = map.get_or_insert(3, 30);
  THES_CHECK(fresh == 30);
  fresh = 33;
  THES_CHECK(map.at(3) == 33);

  // Insertion keeps the map sorted, including when the new key sorts in the middle.
  map.get_or_insert(2, 20);
  THES_CHECK(test::range_eq(map, Entries{{1, 11}, {2, 20}, {3, 33}}));
}

/** Checks that `transform_or_create` takes the right branch for present and absent keys. */
THES_TEST_CASE("transform_or_create updates or creates", "[containers][flat-map]") {
  Map map = make_map({{1, 10}});

  int transforms = 0;
  int creations = 0;
  const auto bump = [&transforms](int& value) {
    ++transforms;
    value += 1;
  };
  const auto create = [&creations] {
    ++creations;
    return 100;
  };

  map.transform_or_create(1, bump, create);
  THES_CHECK(transforms == 1);
  THES_CHECK(creations == 0);
  THES_CHECK(map.at(1) == 11);

  map.transform_or_create(2, bump, create);
  THES_CHECK(transforms == 1);
  THES_CHECK(creations == 1);
  THES_CHECK(map.at(2) == 100);

  map.transform_or_create(2, bump, create);
  THES_CHECK(transforms == 2);
  THES_CHECK(creations == 1);
  THES_CHECK(map.at(2) == 101);

  THES_CHECK(test::range_eq(map, Entries{{1, 11}, {2, 101}}));
}

//==================================================================================================
// Removal
//==================================================================================================

/** Checks that `erase` reports whether it removed anything and keeps the order. */
THES_TEST_CASE("erase removes a single key", "[containers][flat-map]") {
  Map map = make_map({{1, 10}, {2, 20}, {3, 30}});

  THES_CHECK(map.erase(2));
  THES_CHECK(!map.erase(2));
  THES_CHECK(!map.erase(9));
  THES_CHECK(test::range_eq(map, Entries{{1, 10}, {3, 30}}));
}

/** Checks that `erase_if` removes every matching entry at once. */
THES_TEST_CASE("erase_if removes matching entries", "[containers][flat-map]") {
  Map map = make_map({{1, 10}, {2, 20}, {3, 30}, {4, 40}});

  map.erase_if([](const auto& entry) { return entry.first % 2 == 0; });
  THES_CHECK(test::range_eq(map, Entries{{1, 10}, {3, 30}}));

  map.erase_if([](const auto& /*entry*/) { return false; });
  THES_CHECK(map.size() == 2);

  map.erase_if([](const auto& /*entry*/) { return true; });
  THES_CHECK(map.empty());
}

/** Checks that `clear` empties the map without breaking later use. */
THES_TEST_CASE("clear empties the map", "[containers][flat-map]") {
  Map map = make_map({{1, 10}, {2, 20}});

  map.clear();
  THES_CHECK(map.empty());
  THES_CHECK(map.size() == 0);
  THES_CHECK(map.begin() == map.end());
  THES_CHECK(!map.contains(1));

  map.insert(7, 70);
  THES_CHECK(test::range_eq(map, Entries{{7, 70}}));
}

//==================================================================================================
// Custom comparators
//==================================================================================================

/** Checks that a reversed key comparator flips the stored order and lookup alike. */
THES_TEST_CASE("a custom comparator reverses the order", "[containers][flat-map]") {
  thes::FlatMap<int, std::string, std::greater<>> map{};
  map.insert(1, "one");
  map.insert(3, "three");
  map.insert(2, "two");

  THES_REQUIRE(map.size() == 3);
  THES_CHECK(map.front().first == 3);
  THES_CHECK(map.at(2) == "two");
  THES_CHECK(map.contains(1));
  THES_CHECK(!map.contains(4));

  const auto keys = [&map] {
    std::vector<int> out{};
    std::ranges::transform(map, std::back_inserter(out), [](const auto& e) { return e.first; });
    return out;
  }();
  THES_CHECK(test::range_eq(keys, std::vector<int>{3, 2, 1}));
}
} // namespace

THES_TEST_MAIN()
