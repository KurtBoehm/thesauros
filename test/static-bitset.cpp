// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <array>
#include <cstddef>
#include <ranges>
#include <type_traits>

#include "thesauros/containers.hpp"
#include "thesauros/format.hpp"
#include "thesauros/test.hpp"

namespace test = thes::test;

namespace {
//--------------------------------------------------------------------------------------------------
// Compile-time checks that `StaticBitset` is a real, writable range
//--------------------------------------------------------------------------------------------------

template<typename Bitset>
constexpr void check_range_concepts() {
  static_assert(std::ranges::forward_range<Bitset>);
  static_assert(std::ranges::forward_range<const Bitset>);
  static_assert(std::ranges::sized_range<Bitset>);
  static_assert(std::ranges::output_range<Bitset, bool>);
  static_assert(std::same_as<std::ranges::range_value_t<const Bitset>, bool>);
  static_assert(std::same_as<std::ranges::range_reference_t<const Bitset>, bool>);
  static_assert(std::same_as<std::ranges::range_reference_t<Bitset>, typename Bitset::MutBitRef>);
}

//--------------------------------------------------------------------------------------------------
// Construction and basic bit access
//--------------------------------------------------------------------------------------------------

template<typename Bitset>
constexpr void test_construction() {
  const Bitset all_false{false};
  const Bitset all_true{true};
  for (std::size_t i = 0; i < Bitset::static_size; ++i) {
    THES_ALWAYS_ASSERT(!all_false.get(i));
    THES_ALWAYS_ASSERT(all_true.get(i));
  }
}

//--------------------------------------------------------------------------------------------------
// Range-based mutation and comparison against a `std::array` reference
//--------------------------------------------------------------------------------------------------

template<std::size_t Size, std::size_t ChunkByteNum>
constexpr void test_range_behavior() {
  using Bitset = thes::StaticBitset<Size, ChunkByteNum>;
  check_range_concepts<Bitset>();

  std::array<bool, Size> ref{};
  Bitset bitset{false};

  auto assert_eq = [&] {
    if (!std::is_constant_evaluated()) {
      fmt::print("{}\n", bitset);
    }
    THES_ALWAYS_ASSERT(test::range_eq(bitset, ref));
    THES_ALWAYS_ASSERT(std::ranges::equal(bitset, ref));
  };
  assert_eq();

  // `set`/`unset` on alternating indices.
  for (std::size_t i = 0; i < Size; i += 2) {
    bitset.set(i);
    ref[i] = true;
  }
  assert_eq();
  for (std::size_t i = 0; i < Size; i += 2) {
    bitset.unset(i);
    ref[i] = false;
  }
  assert_eq();

  // Mutation through the writable range interface.
  std::size_t idx = 0;
  for (auto bit : bitset) {
    const bool value = idx % 3 == 0;
    bit = value;
    ref[idx] = value;
    ++idx;
  }
  assert_eq();

  // `operator[]` should agree with what iteration reports.
  for (std::size_t i = 0; i < Size; ++i) {
    THES_ALWAYS_ASSERT(bitset[i] == ref[i]);
    bitset[i] = !ref[i];
    ref[i] = !ref[i];
  }
  assert_eq();

  // Standard range algorithms should work directly on the bitset.
  std::ranges::fill(bitset, true);
  std::ranges::fill(ref, true);
  assert_eq();
  THES_ALWAYS_ASSERT(static_cast<std::size_t>(std::ranges::count(bitset, true)) == Size);

  std::ranges::fill(bitset, false);
  std::ranges::fill(ref, false);
  assert_eq();
  THES_ALWAYS_ASSERT(static_cast<std::size_t>(std::ranges::count(bitset, true)) == 0);

  bitset.fill(true);
  ref.fill(true);
  assert_eq();
}

//--------------------------------------------------------------------------------------------------
// `countr_zero`/`countr_one` edge cases
//--------------------------------------------------------------------------------------------------

template<std::size_t Size, std::size_t ChunkByteNum>
constexpr void test_countr() {
  using Bitset = thes::StaticBitset<Size, ChunkByteNum>;

  const Bitset all_false{false};
  THES_ALWAYS_ASSERT(all_false.countr_zero() == Size);
  THES_ALWAYS_ASSERT(all_false.countr_one() == 0);

  const Bitset all_true{true};
  THES_ALWAYS_ASSERT(all_true.countr_one() == Size);
  THES_ALWAYS_ASSERT(all_true.countr_zero() == 0);

  if constexpr (Size > 0) {
    // A single unset bit at the front should cap `countr_one` at zero.
    Bitset front_unset{true};
    front_unset.unset(0);
    THES_ALWAYS_ASSERT(front_unset.countr_one() == 0);
    THES_ALWAYS_ASSERT(front_unset.countr_zero() == 1);

    // A single set bit at the front should cap `countr_zero` at zero.
    Bitset front_set{false};
    front_set.set(0);
    THES_ALWAYS_ASSERT(front_set.countr_zero() == 0);
    THES_ALWAYS_ASSERT(front_set.countr_one() == 1);

    // Flipping the last bit should not affect a `countr_*` count that stops earlier.
    Bitset last_flipped{false};
    last_flipped.set(Size - 1);
    THES_ALWAYS_ASSERT(last_flipped.countr_zero() == Size - 1);
  }
}

//--------------------------------------------------------------------------------------------------
// Full suite for one `(Size, ChunkByteNum)` combination
//--------------------------------------------------------------------------------------------------

template<std::size_t Size, std::size_t ChunkByteNum>
constexpr void test_bitset() {
  using Bitset = thes::StaticBitset<Size, ChunkByteNum>;
  test_construction<Bitset>();
  test_range_behavior<Size, ChunkByteNum>();
  test_countr<Size, ChunkByteNum>();
}

//--------------------------------------------------------------------------------------------------
// The original scenario: a long, hand-written bit pattern exercised through `set`/`unset`/`get`
//--------------------------------------------------------------------------------------------------

constexpr void test_pattern() {
  thes::StaticBitset bitset{
    false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  false,
    true,  false, true,  false, true,  false, true,  false, true,  false, true,  false, true,
    false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  false,
    true,  false, true,  false, true,  false, true,  false, true,  false, true,  false, true,
    false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  true,
  };
  std::array ref{
    false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  false,
    true,  false, true,  false, true,  false, true,  false, true,  false, true,  false, true,
    false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  false,
    true,  false, true,  false, true,  false, true,  false, true,  false, true,  false, true,
    false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  true,
  };

  auto assert_eq = [&] {
    if (!std::is_constant_evaluated()) {
      fmt::print("{}\n", bitset);
    }
    THES_ALWAYS_ASSERT(test::range_eq(bitset, ref));
  };

  auto set = [&](std::size_t idx) {
    bitset.set(idx);
    ref[idx] = true;
    assert_eq();
  };
  auto unset = [&](std::size_t idx) {
    bitset.unset(idx);
    ref[idx] = false;
    assert_eq();
  };
  auto get = [&](std::size_t idx) {
    const bool v1 = bitset.get(idx);
    const bool v2 = ref[idx];
    THES_ALWAYS_ASSERT(v1 == v2);
    if (!std::is_constant_evaluated()) {
      fmt::print("@{}: {}\n", idx, v1);
    }
  };
  auto countr_one = [&] {
    const auto v1 = bitset.countr_one();
    const auto v2 = [&] {
      std::size_t count = 0;
      for (const auto v : ref) {
        if (!v) {
          break;
        }
        ++count;
      }
      return count;
    }();
    THES_ALWAYS_ASSERT(v1 == v2);
  };
  auto countr_zero = [&] {
    const auto v1 = bitset.countr_zero();
    const auto v2 = [&] {
      std::size_t count = 0;
      for (const auto v : ref) {
        if (v) {
          break;
        }
        ++count;
      }
      return count;
    }();
    THES_ALWAYS_ASSERT(v1 == v2);
  };

  assert_eq();
  set(2);
  unset(bitset.size() - 1);
  get(1);
  countr_one();
  countr_zero();
  unset(1);
  countr_one();
  countr_zero();
}

constexpr int run() {
  // Chunks of a single byte, exercising sizes below, at, and above the chunk boundary.
  test_bitset<1, 1>();
  test_bitset<7, 1>();
  test_bitset<8, 1>();
  test_bitset<9, 1>();
  // Wider chunks, again spanning chunk boundaries.
  test_bitset<63, 8>();
  test_bitset<64, 8>();
  test_bitset<65, 8>();
  test_bitset<128, 4>();

  test_pattern();
  return 0;
}
} // namespace

int main() {
  static_assert(run() == 0);
  return run();
}
