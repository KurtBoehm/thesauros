// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <cstddef>
#include <iterator>
#include <numeric>
#include <vector>

#include "thesauros/random/lcg.hpp"
#include "thesauros/test/test.hpp"
#include "thesauros/types/primitives.hpp"

namespace {
using Lcg = thes::LCG<thes::u32>;

/** Collects the values an `LCG` produces, in order. */
[[nodiscard]] std::vector<thes::u32> collect(const Lcg& lcg) {
  std::vector<thes::u32> values{};
  for (auto value : lcg) {
    values.push_back(value);
  }
  return values;
}

/** Returns whether `values` is a permutation of `0, …, size - 1`. */
[[nodiscard]] bool is_permutation_of_indices(const std::vector<thes::u32>& values, thes::u32 size) {
  if (values.size() != size) {
    return false;
  }
  std::vector<bool> seen(size, false);
  for (thes::u32 value : values) {
    if (value >= size || seen[value]) {
      return false;
    }
    seen[value] = true;
  }
  return true;
}

//==================================================================================================
// Basic properties
//==================================================================================================

/** Checks the accessors and that the sequence starts at the seed and has exactly `size` steps. */
THES_TEST_CASE("an LCG starts at its seed and has the requested length", "[random][lcg]") {
  const Lcg lcg{3, 7, 20};

  THES_CHECK(lcg.seed() == 3);
  THES_CHECK(lcg.increment() == 7);
  THES_CHECK(*lcg.begin() == 3);
  THES_CHECK(lcg.begin().index() == 0);
  THES_CHECK(lcg.end().index() == 20);
  THES_CHECK(lcg.end() - lcg.begin() == 20);
  THES_CHECK(collect(lcg).size() == 20);
}

/** Checks that each step advances by the increment, modulo the size. */
THES_TEST_CASE("each step adds the increment modulo the size", "[random][lcg]") {
  static constexpr thes::u32 size = 20;
  static constexpr thes::u32 seed = 3;
  static constexpr thes::u32 increment = 7;

  const auto values = collect(Lcg{seed, increment, size});
  THES_REQUIRE(values.size() == size);
  THES_CHECK(values[0] == seed);
  for (std::size_t i = 1; i < values.size(); ++i) {
    THES_CHECK(values[i] == (values[i - 1] + increment) % size);
  }
}

/**
 * Checks that the generator enumerates every index exactly once whenever the increment is coprime
 * to the size, which is the property that makes it usable as a permutation.
 */
THES_TEST_CASE("a coprime increment yields a permutation", "[random][lcg]") {
  static constexpr thes::u32 size = 30;

  for (thes::u32 increment = 1; increment < size; ++increment) {
    if (std::gcd(increment, size) != 1) {
      continue;
    }
    for (thes::u32 seed = 0; seed < size; seed += 7) {
      const auto values = collect(Lcg{seed, increment, size});
      THES_CHECK(is_permutation_of_indices(values, size));
    }
  }
}

/** Checks the degenerate sizes, where the sequence is empty or has a single element. */
THES_TEST_CASE("degenerate sizes", "[random][lcg]") {
  const auto empty = collect(Lcg{0, 1, 0});
  THES_CHECK(empty.empty());

  const auto single = collect(Lcg{0, 1, 1});
  THES_CHECK(single.size() == 1);
  THES_CHECK(single[0] == 0);

  // An increment of zero repeats the seed.
  const auto constant = collect(Lcg{4, 0, 5});
  THES_CHECK(constant.size() == 5);
  for (thes::u32 value : constant) {
    THES_CHECK(value == 4);
  }
}

//==================================================================================================
// Iterator arithmetic
//==================================================================================================

/**
 * Checks that jumping by `n` with `operator+` lands on the same value as stepping `n` times, which
 * exercises the doubling loop in the iterator’s skip computation.
 */
THES_TEST_CASE("jumping matches repeated stepping", "[random][lcg][iterator]") {
  static constexpr thes::u32 size = 37;
  const Lcg lcg{5, 11, size};
  const auto values = collect(lcg);
  THES_REQUIRE(values.size() == size);

  for (std::ptrdiff_t offset = 0; offset <= std::ptrdiff_t{size}; ++offset) {
    auto it = lcg.begin() + offset;
    THES_CHECK(it.index() == thes::u32(offset));
    if (offset < std::ptrdiff_t{size}) {
      THES_CHECK(*it == values[std::size_t(offset)]);
    }
  }
}

/** Checks that stepping backwards from an offset undoes the forward jump. */
THES_TEST_CASE("backwards jumps undo forwards jumps", "[random][lcg][iterator]") {
  static constexpr thes::u32 size = 37;
  const Lcg lcg{5, 11, size};
  const auto values = collect(lcg);

  for (std::ptrdiff_t offset = 1; offset < std::ptrdiff_t{size}; ++offset) {
    auto it = lcg.begin() + offset;

    auto back = it - offset;
    THES_CHECK(back.index() == 0);
    THES_CHECK(*back == lcg.seed());

    --it;
    THES_CHECK(it.index() == thes::u32(offset - 1));
    THES_CHECK(*it == values[std::size_t(offset - 1)]);
  }
}

/** Checks the comparison and difference operators derived from the iterator’s index. */
THES_TEST_CASE("iterator comparison follows the index", "[random][lcg][iterator]") {
  const Lcg lcg{2, 3, 10};

  auto first = lcg.begin();
  auto second = first + 4;

  THES_CHECK(second - first == 4);
  THES_CHECK(first - second == -4);
  THES_CHECK(first < second);
  THES_CHECK(second > first);
  THES_CHECK(first <= first);
  THES_CHECK(first >= first);
  THES_CHECK(first != second);
  THES_CHECK(first == lcg.begin());

  second -= 4;
  THES_CHECK(second == first);
  second += 10;
  THES_CHECK(second == lcg.end());
}

/** Checks that the iterator works with the standard algorithms via `std::distance`. */
THES_TEST_CASE("the LCG is a random access range", "[random][lcg][iterator]") {
  const Lcg lcg{1, 4, 9};
  THES_CHECK(std::distance(lcg.begin(), lcg.end()) == 9);
  THES_CHECK(lcg.begin()[3] == *(lcg.begin() + 3));
}
} // namespace

THES_TEST_MAIN()
