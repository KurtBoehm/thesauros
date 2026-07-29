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
#include <utility>
#include <vector>

#include "thesauros/algorithms/sort-indices.hpp"
#include "thesauros/test/equality.hpp"
#include "thesauros/test/test.hpp"

namespace test = thes::test;

namespace {
/** Sorts `values` in place, using `sort_indices` with the natural swap. */
void sort_in_place(std::vector<int>& values, auto comp) {
  thes::sort_indices(values.data(), values.size(), comp,
                     [&values](std::size_t i, std::size_t j) { std::swap(values[i], values[j]); });
}

//==================================================================================================
// Compile-time evaluation
//==================================================================================================

/** Sorts a `std::array` at compile time, checking that `sort_indices` is usable in constexpr. */
consteval std::array<int, 5> sorted_at_compile_time() {
  std::array<int, 5> values{4, 1, 5, 2, 3};
  thes::sort_indices(values.data(), values.size(), std::less<>{},
                     [&values](std::size_t i, std::size_t j) { std::swap(values[i], values[j]); });
  return values;
}
static_assert(sorted_at_compile_time() == std::array<int, 5>{1, 2, 3, 4, 5});

//==================================================================================================
// Sorting
//==================================================================================================

/** Checks the sizes at which insertion sort has no work to do. */
THES_TEST_CASE("empty and single-element ranges are left alone", "[algorithms][sort-indices]") {
  std::vector<int> empty{};
  sort_in_place(empty, std::less<>{});
  THES_CHECK(empty.empty());

  std::vector<int> single{7};
  sort_in_place(single, std::less<>{});
  THES_CHECK(test::range_eq(single, std::vector<int>{7}));
}

/** Checks sorting of already sorted, reversed and duplicate-laden inputs. */
THES_TEST_CASE("sorting handles ordered, reversed and repeated input",
               "[algorithms][sort-indices]") {
  {
    std::vector<int> values{1, 2, 3, 4, 5};
    sort_in_place(values, std::less<>{});
    THES_CHECK(test::range_eq(values, std::vector<int>{1, 2, 3, 4, 5}));
  }
  {
    std::vector<int> values{5, 4, 3, 2, 1};
    sort_in_place(values, std::less<>{});
    THES_CHECK(test::range_eq(values, std::vector<int>{1, 2, 3, 4, 5}));
  }
  {
    std::vector<int> values{3, 3, 1, 3, 1};
    sort_in_place(values, std::less<>{});
    THES_CHECK(test::range_eq(values, std::vector<int>{1, 1, 3, 3, 3}));
  }
}

/** Checks that a reversed comparator produces a descending order. */
THES_TEST_CASE("a custom comparator changes the order", "[algorithms][sort-indices]") {
  std::vector<int> values{2, 5, 1, 4, 3};
  sort_in_place(values, std::greater<>{});
  THES_CHECK(test::range_eq(values, std::vector<int>{5, 4, 3, 2, 1}));
}

/** Checks `sort_indices` against `std::ranges::sort` over every permutation of five elements. */
THES_TEST_CASE("sorting matches the standard algorithm on all permutations",
               "[algorithms][sort-indices]") {
  std::vector<int> permutation{0, 1, 2, 3, 4};
  std::ranges::sort(permutation);

  for (bool next = true; next; next = std::ranges::next_permutation(permutation).found) {
    std::vector<int> values = permutation;
    sort_in_place(values, std::less<>{});
    THES_CHECK(test::range_eq(values, std::vector<int>{0, 1, 2, 3, 4}));
  }
}

//==================================================================================================
// Permuting a separate array
//==================================================================================================

/**
 * Checks the intended use of `sort_indices`: the swap callback receives indices, so it can reorder
 * one or more parallel arrays alongside the one being compared.
 */
THES_TEST_CASE("the swap callback can reorder parallel arrays", "[algorithms][sort-indices]") {
  std::vector<int> keys{30, 10, 40, 20};
  std::vector<char> payload{'c', 'a', 'd', 'b'};
  std::vector<std::size_t> order(keys.size());
  std::ranges::iota(order, std::size_t{0});

  thes::sort_indices(keys.data(), keys.size(), std::less<>{}, [&](std::size_t i, std::size_t j) {
    std::swap(keys[i], keys[j]);
    std::swap(payload[i], payload[j]);
    std::swap(order[i], order[j]);
  });

  THES_CHECK(test::range_eq(keys, std::vector<int>{10, 20, 30, 40}));
  THES_CHECK(test::range_eq(payload, std::vector<char>{'a', 'b', 'c', 'd'}));
  // `order` now holds, for each sorted position, the index it originally came from.
  THES_CHECK(test::range_eq(order, std::vector<std::size_t>{1, 3, 0, 2}));
}

/** Checks that insertion sort is stable, i.e. equal elements keep their relative order. */
THES_TEST_CASE("sorting is stable", "[algorithms][sort-indices]") {
  // The keys are compared, the tags only carried along; equal keys must keep their tag order.
  std::vector<int> keys{2, 1, 2, 1, 2};
  std::vector<int> tags{0, 1, 2, 3, 4};

  thes::sort_indices(keys.data(), keys.size(), std::less<>{}, [&](std::size_t i, std::size_t j) {
    std::swap(keys[i], keys[j]);
    std::swap(tags[i], tags[j]);
  });

  THES_CHECK(test::range_eq(keys, std::vector<int>{1, 1, 2, 2, 2}));
  THES_CHECK(test::range_eq(tags, std::vector<int>{1, 3, 0, 2, 4}));
}
} // namespace

THES_TEST_MAIN()
