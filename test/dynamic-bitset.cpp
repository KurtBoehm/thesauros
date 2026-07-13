// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <ranges>
#include <thread>
#include <utility>

#include "thesauros/containers.hpp"
#include "thesauros/ranges.hpp"
#include "thesauros/test.hpp"

namespace test = thes::test;

namespace {
//--------------------------------------------------------------------------------------------------
// Range concept checks
//--------------------------------------------------------------------------------------------------

static_assert(std::ranges::sized_range<thes::DynamicBitset<std::uint32_t>>);
static_assert(std::ranges::random_access_range<thes::DynamicBitset<std::uint32_t>>);
static_assert(std::ranges::random_access_range<const thes::DynamicBitset<std::uint32_t>>);
static_assert(std::ranges::output_range<thes::DynamicBitset<std::uint32_t>, bool>);
static_assert(
  std::same_as<std::ranges::range_value_t<const thes::DynamicBitset<std::uint32_t>>, bool>);

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

void test_construction() {
  // Default construction: empty.
  thes::DynamicBitset<std::uint32_t> b1{};
  THES_ALWAYS_ASSERT(b1.chunk_num() == 0 && b1.size() == 0);

  // Size-only construction: correct size/chunk_num, values unspecified.
  thes::DynamicBitset<std::uint32_t> b2{10};
  THES_ALWAYS_ASSERT(b2.size() == 10 && b2.chunk_num() == 1);

  // Size + value = true, spanning multiple chunks.
  thes::DynamicBitset<std::uint32_t> b3{40, true};
  THES_ALWAYS_ASSERT(b3.size() == 40 && b3.chunk_num() == 2);
  THES_ALWAYS_ASSERT(std::ranges::all_of(b3, [](bool v) { return v; }));
  THES_ALWAYS_ASSERT(b3.countr_one() == 40 && b3.countr_zero() == 0);

  // Size + value = false, spanning multiple chunks.
  thes::DynamicBitset<std::uint32_t> b4{40, false};
  THES_ALWAYS_ASSERT(b4.size() == 40 && b4.chunk_num() == 2);
  THES_ALWAYS_ASSERT(std::ranges::none_of(b4, [](bool v) { return v; }));
  THES_ALWAYS_ASSERT(b4.countr_zero() == 40 && b4.countr_one() == 0);
}

//--------------------------------------------------------------------------------------------------
// `push_back` and string formatting
//--------------------------------------------------------------------------------------------------

void test_push_back_and_string() {
  thes::DynamicBitset<std::uint32_t> bitset{};
  THES_ALWAYS_ASSERT(bitset.chunk_num() == 0 && bitset.size() == 0);
  for (const auto i : thes::range<std::size_t>(32)) {
    bitset.push_back((i % 2) != 0);
  }
  THES_ALWAYS_ASSERT(bitset.chunk_num() == 1 && bitset.countr_one() == 0 &&
                     bitset.countr_zero() == 1);
  THES_ALWAYS_ASSERT(test::string_eq("10101010101010101010101010101010", bitset));

  bitset.push_back(true);
  bitset.set(0);
  THES_ALWAYS_ASSERT(bitset.chunk_num() == 2 && bitset.countr_one() == 2 &&
                     bitset.countr_zero() == 0);
  THES_ALWAYS_ASSERT(test::string_eq("110101010101010101010101010101011", bitset));

  for (const auto i : thes::range<std::size_t>(31)) {
    bitset.push_back((i % 2) != 0);
  }
  THES_ALWAYS_ASSERT(bitset.chunk_num() == 2 && bitset.countr_one() == 2 &&
                     bitset.countr_zero() == 0);
  THES_ALWAYS_ASSERT(
    test::string_eq("0101010101010101010101010101010110101010101010101010101010101011", bitset));

  bitset.push_back(true);
  THES_ALWAYS_ASSERT(bitset.chunk_num() == 3 && bitset.countr_one() == 2 &&
                     bitset.countr_zero() == 0);
  THES_ALWAYS_ASSERT(
    test::string_eq("10101010101010101010101010101010110101010101010101010101010101011", bitset));
}

//--------------------------------------------------------------------------------------------------
// `set`/`unset`/`get`
//--------------------------------------------------------------------------------------------------

void test_set_unset_get() {
  thes::DynamicBitset<std::uint32_t> b{64, false};
  b.set(0);
  b.set(31);
  b.set(63);
  THES_ALWAYS_ASSERT(b.get(0) && b.get(31) && b.get(63));
  THES_ALWAYS_ASSERT(!b.get(1) && !b.get(32));

  b.unset(0);
  THES_ALWAYS_ASSERT(!b.get(0));
  b.unset(63);
  THES_ALWAYS_ASSERT(!b.get(63));
  // Unrelated bit is unaffected.
  THES_ALWAYS_ASSERT(b.get(31));

  // unset() on an already-unset bit is a no-op.
  b.unset(0);
  THES_ALWAYS_ASSERT(!b.get(0));
}

void test_set_if_unset() {
  thes::DynamicBitset<std::uint32_t> b{10, false};
  // First call: bit was unset, gets set, returns true.
  THES_ALWAYS_ASSERT(b.set_if_unset(5));
  THES_ALWAYS_ASSERT(b.get(5));
  // Second call: bit already set, returns false, stays set.
  THES_ALWAYS_ASSERT(!b.set_if_unset(5));
  THES_ALWAYS_ASSERT(b.get(5));
  // Unrelated bits remain untouched.
  THES_ALWAYS_ASSERT(!b.get(4) && !b.get(6));
}

//--------------------------------------------------------------------------------------------------
// `clear` and `resize`
//--------------------------------------------------------------------------------------------------

void test_clear() {
  thes::DynamicBitset<std::uint32_t> b{50, true};
  THES_ALWAYS_ASSERT(b.size() == 50 && b.chunk_num() == 2);
  b.clear();
  THES_ALWAYS_ASSERT(b.size() == 0 && b.chunk_num() == 0);
  // Bitset remains usable after clear().
  b.push_back(true);
  THES_ALWAYS_ASSERT(b.size() == 1 && b.get(0));
}

void test_resize() {
  thes::DynamicBitset<std::uint32_t> b{};
  b.resize(20);
  THES_ALWAYS_ASSERT(b.size() == 20 && b.chunk_num() == 1);

  b.resize(100);
  THES_ALWAYS_ASSERT(b.size() == 100 && b.chunk_num() == 4);

  // Shrinking reduces size/chunk_num correctly.
  b.resize(5);
  THES_ALWAYS_ASSERT(b.size() == 5 && b.chunk_num() == 1);

  b.resize(0);
  THES_ALWAYS_ASSERT(b.size() == 0 && b.chunk_num() == 0);
}

//--------------------------------------------------------------------------------------------------
// `fill` and `operator[]`
//--------------------------------------------------------------------------------------------------

void test_fill() {
  thes::DynamicBitset<std::uint32_t> b{70};
  b.fill(true);
  THES_ALWAYS_ASSERT(std::ranges::all_of(std::as_const(b), [](bool v) { return v; }));
  THES_ALWAYS_ASSERT(b.countr_one() == 70);

  b.fill(false);
  THES_ALWAYS_ASSERT(std::ranges::none_of(std::as_const(b), [](bool v) { return v; }));
  THES_ALWAYS_ASSERT(b.countr_zero() == 70);
}

void test_operator_bracket() {
  thes::DynamicBitset<std::uint32_t> b{20, false};
  const auto& cb = b;
  THES_ALWAYS_ASSERT(!cb[3]);

  b[3] = true;
  THES_ALWAYS_ASSERT(cb[3] && b.get(3));

  b[3] = false;
  THES_ALWAYS_ASSERT(!cb[3] && !b.get(3));

  // Exercise a bit in the second word too.
  b[17] = true;
  THES_ALWAYS_ASSERT(cb[17]);
}

//--------------------------------------------------------------------------------------------------
// `countr_zero`/`countr_one` edge cases
//--------------------------------------------------------------------------------------------------

void test_countr_edge_cases() {
  // Empty bitset: both counts are zero.
  thes::DynamicBitset<std::uint32_t> b0{};
  THES_ALWAYS_ASSERT(b0.countr_zero() == 0 && b0.countr_one() == 0);

  // Bitset shorter than a single chunk.
  thes::DynamicBitset<std::uint32_t> b1{10, false};
  THES_ALWAYS_ASSERT(b1.countr_zero() == 10 && b1.countr_one() == 0);

  thes::DynamicBitset<std::uint32_t> b2{10, true};
  THES_ALWAYS_ASSERT(b2.countr_one() == 10 && b2.countr_zero() == 0);

  // Exactly two full chunks, uniform value.
  thes::DynamicBitset<std::uint32_t> b3{64, false};
  THES_ALWAYS_ASSERT(b3.countr_zero() == 64);

  thes::DynamicBitset<std::uint32_t> b4{64, true};
  THES_ALWAYS_ASSERT(b4.countr_one() == 64);

  // Run stops exactly at a chunk boundary.
  thes::DynamicBitset<std::uint32_t> b5{64, true};
  b5.unset(32);
  THES_ALWAYS_ASSERT(b5.countr_one() == 32);

  thes::DynamicBitset<std::uint32_t> b6{64, false};
  b6.set(32);
  THES_ALWAYS_ASSERT(b6.countr_zero() == 32);

  // Run stops one bit before the very end.
  thes::DynamicBitset<std::uint32_t> b7{64, true};
  b7.unset(63);
  THES_ALWAYS_ASSERT(b7.countr_one() == 63);
}

//--------------------------------------------------------------------------------------------------
// Iteration
//--------------------------------------------------------------------------------------------------

void test_iteration() {
  thes::DynamicBitset<std::uint32_t> b{};
  b.push_back(true);
  b.push_back(false);
  b.push_back(true);

  static constexpr std::array expected{true, false, true};
  THES_ALWAYS_ASSERT(std::ranges::equal(std::as_const(b), expected));
}

/** Exercises the writable range interface, not just `set`/`unset`/`operator[]`. */
void test_mutable_iteration() {
  thes::DynamicBitset<std::uint32_t> b{16, false};

  std::size_t idx = 0;
  for (auto bit : b) {
    bit = (idx % 3 == 0);
    ++idx;
  }
  for (std::size_t i = 0; i < 16; ++i) {
    THES_ALWAYS_ASSERT(b.get(i) == (i % 3 == 0));
  }

  std::ranges::fill(b, true);
  THES_ALWAYS_ASSERT(std::ranges::all_of(std::as_const(b), [](bool v) { return v; }));
}

void test_random_access_iterator() {
  thes::DynamicBitset<std::uint32_t> b{};
  static constexpr std::array values{true, false, true, true, false, false, true, false};
  for (const bool v : values) {
    b.push_back(v);
  }

  // Read through a `const` view, guaranteeing `bool` rather than the writable proxy reference.
  const auto& cb = b;
  const auto begin = cb.begin();
  const auto end = cb.end();

  // distance / size relationship.
  THES_ALWAYS_ASSERT(std::distance(begin, end) == 8);
  THES_ALWAYS_ASSERT(static_cast<std::size_t>(end - begin) == cb.size());

  // operator[] random access, without moving the base iterator.
  for (const auto i : thes::range<std::size_t>(values.size())) {
    THES_ALWAYS_ASSERT(begin[*thes::safe_cast<std::ptrdiff_t>(i)] == values[i]);
  }

  // operator+ / operator+= jump forward from begin.
  auto it = begin + 3;
  THES_ALWAYS_ASSERT(*it == values[3]);
  it += 2;
  THES_ALWAYS_ASSERT(*it == values[5]);

  // operator- / operator-= jump backward, including from end().
  auto it2 = end - 1;
  THES_ALWAYS_ASSERT(*it2 == values[7]);
  it2 -= 3;
  THES_ALWAYS_ASSERT(*it2 == values[4]);

  // Difference between two iterators.
  THES_ALWAYS_ASSERT((it - begin) == 5);
  THES_ALWAYS_ASSERT((end - it2) == 4);

  // Relational operators.
  THES_ALWAYS_ASSERT(begin < end);
  THES_ALWAYS_ASSERT(end > begin);
  THES_ALWAYS_ASSERT(begin <= begin);
  THES_ALWAYS_ASSERT(end >= end);
  THES_ALWAYS_ASSERT((begin + 3) < (begin + 5));
  THES_ALWAYS_ASSERT((begin + 5) > (begin + 3));

  // Pre/post increment and decrement.
  auto it3 = begin;
  THES_ALWAYS_ASSERT(*it3 == values[0]);
  ++it3;
  THES_ALWAYS_ASSERT(*it3 == values[1]);
  auto it3_post = it3++;
  THES_ALWAYS_ASSERT(*it3_post == values[1] && *it3 == values[2]);
  --it3;
  THES_ALWAYS_ASSERT(*it3 == values[1]);
  auto it3_post_dec = it3--;
  THES_ALWAYS_ASSERT(*it3_post_dec == values[1] && *it3 == values[0]);

  // Iterating backward from end() to begin() via decrement reproduces the sequence in reverse.
  THES_ALWAYS_ASSERT(std::ranges::equal(std::views::reverse(cb), std::views::reverse(values)));

  // Equality/inequality.
  THES_ALWAYS_ASSERT(begin == cb.begin());
  THES_ALWAYS_ASSERT(begin != end);
}

//--------------------------------------------------------------------------------------------------
// Non-default chunk types
//--------------------------------------------------------------------------------------------------

void test_different_chunk_types() {
  // A chunk type smaller than a word stresses div_ceil/masking differently.
  {
    thes::DynamicBitset<std::uint8_t> b{20, true};
    THES_ALWAYS_ASSERT(b.chunk_num() == 3 && b.size() == 20);
    THES_ALWAYS_ASSERT(b.countr_one() == 20);
    b.unset(7);
    THES_ALWAYS_ASSERT(b.countr_one() == 7);
  }
  // A wider chunk type.
  {
    thes::DynamicBitset<std::uint64_t> b{200, false};
    THES_ALWAYS_ASSERT(b.chunk_num() == 4 && b.size() == 200);
    THES_ALWAYS_ASSERT(b.countr_zero() == 200);
    b.set(199);
    THES_ALWAYS_ASSERT(b.countr_zero() == 199);
  }
}

//--------------------------------------------------------------------------------------------------
// Concurrent `set_if_unset`
//--------------------------------------------------------------------------------------------------

void test_concurrent_set_if_unset_disjoint() {
  // Two threads claim disjoint index ranges: every claim must succeed exactly
  // once, and no cross-thread interference should occur.
  static constexpr std::size_t total = 100000;
  thes::DynamicBitset<std::uint64_t> b{total, false};

  std::array<std::atomic<std::size_t>, 2> claimed{};

  auto worker = [&](std::size_t start, std::size_t stop, std::atomic<std::size_t>& counter) {
    for (std::size_t i = start; i < stop; ++i) {
      if (b.set_if_unset(i)) {
        ++counter;
      }
    }
  };

  {
    std::jthread t0{worker, std::size_t{0}, total / 2, std::ref(claimed[0])};
    std::jthread t1{worker, total / 2, total, std::ref(claimed[1])};
  }

  THES_ALWAYS_ASSERT(claimed[0].load() == total / 2);
  THES_ALWAYS_ASSERT(claimed[1].load() == total - total / 2);
  THES_ALWAYS_ASSERT(std::ranges::all_of(std::as_const(b), [](bool v) { return v; }));
}

void test_concurrent_set_if_unset_overlapping() {
  // Two threads race over the *same* index range: set_if_unset() must give
  // each index to exactly one thread, with no double-counting and no bit
  // left unset.
  static constexpr std::size_t total = 100000;
  thes::DynamicBitset<std::uint64_t> b{total, false};

  std::array<std::atomic<std::size_t>, 2> claimed{};

  auto worker = [&](std::atomic<std::size_t>& counter) {
    for (std::size_t i = 0; i < total; ++i) {
      if (b.set_if_unset(i)) {
        ++counter;
      }
    }
  };

  {
    std::jthread t0{worker, std::ref(claimed[0])};
    std::jthread t1{worker, std::ref(claimed[1])};
  }

  // Exactly one thread should have won each of the `total` indices.
  THES_ALWAYS_ASSERT(claimed[0].load() + claimed[1].load() == total);
  // Every bit must end up set, regardless of who claimed it.
  THES_ALWAYS_ASSERT(std::ranges::all_of(std::as_const(b), [](bool v) { return v; }));
}
} // namespace

int main() {
  test_construction();
  test_push_back_and_string();
  test_set_unset_get();
  test_set_if_unset();
  test_clear();
  test_resize();
  test_fill();
  test_operator_bracket();
  test_countr_edge_cases();
  test_iteration();
  test_mutable_iteration();
  test_random_access_iterator();
  test_different_chunk_types();
  test_concurrent_set_if_unset_disjoint();
  test_concurrent_set_if_unset_overlapping();
}
