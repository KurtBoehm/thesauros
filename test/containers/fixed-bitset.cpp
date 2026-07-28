// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <array>
#include <cstddef>
#include <ranges>
#include <vector>

#include "thesauros/containers.hpp"
#include "thesauros/format.hpp"
#include "thesauros/test.hpp"

namespace test = thes::test;

namespace {
//--------------------------------------------------------------------------------------------------
// Compile-time checks that `FixedBitset` is a real, writable range
//--------------------------------------------------------------------------------------------------

template<typename Bitset>
constexpr void check_range_concepts() {
  static_assert(std::ranges::sized_range<Bitset>);
  static_assert(std::ranges::random_access_range<Bitset>);
  static_assert(std::ranges::random_access_range<const Bitset>);
  static_assert(std::ranges::output_range<Bitset, bool>);
  static_assert(std::same_as<std::ranges::range_value_t<const Bitset>, bool>);
  static_assert(std::same_as<std::ranges::range_reference_t<const Bitset>, bool>);
  static_assert(std::same_as<std::ranges::range_reference_t<Bitset>, typename Bitset::MutBitRef>);
}

//--------------------------------------------------------------------------------------------------
// `set`/`unset`/`get`, checked against a `std::vector<bool>` reference
//--------------------------------------------------------------------------------------------------

template<std::size_t ChunkByteNum>
void test_set_unset_get(std::size_t size) {
  thes::FixedBitset<ChunkByteNum> bitset{size};
  std::vector<bool> ref(size, false);

  auto assert_eq = [&] {
    THES_ALWAYS_ASSERT(bitset.size() == ref.size());
    THES_ALWAYS_ASSERT(test::range_eq(bitset, ref));
  };
  assert_eq();

  for (std::size_t i = 0; i < size; i += 2) {
    bitset.set(i);
    ref[i] = true;
  }
  assert_eq();

  for (std::size_t i = 1; i < size; i += 3) {
    bitset.unset(i);
    ref[i] = false;
  }
  assert_eq();

  for (std::size_t i = 0; i < size; ++i) {
    THES_ALWAYS_ASSERT(bitset.get(i) == ref[i]);
    THES_ALWAYS_ASSERT(bitset[i] == ref[i]);
  }

  if (size > 0) {
    THES_ALWAYS_ASSERT(!bitset.empty());
    THES_ALWAYS_ASSERT(bitset.front() == ref.front());
    THES_ALWAYS_ASSERT(bitset.back() == ref.back());
  } else {
    THES_ALWAYS_ASSERT(bitset.empty());
  }
}

//--------------------------------------------------------------------------------------------------
// `fill` and `chunk_num`
//--------------------------------------------------------------------------------------------------

template<std::size_t ChunkByteNum>
void test_fill(std::size_t size) {
  using Bitset = thes::FixedBitset<ChunkByteNum>;
  constexpr std::size_t chunk_bit_num = Bitset::chunk_bit_num;

  Bitset bitset{size, true};
  for (const bool bit : std::as_const(bitset)) {
    THES_ALWAYS_ASSERT(bit);
  }

  bitset.fill(false);
  for (const bool bit : std::as_const(bitset)) {
    THES_ALWAYS_ASSERT(!bit);
  }

  THES_ALWAYS_ASSERT(bitset.chunk_num() == (size + chunk_bit_num - 1) / chunk_bit_num);
}

//--------------------------------------------------------------------------------------------------
// Mutation through the writable range interface, not just `set`/`unset`/`operator[]`
//--------------------------------------------------------------------------------------------------

template<std::size_t ChunkByteNum>
void test_mutable_range(std::size_t size) {
  thes::FixedBitset<ChunkByteNum> bitset{size, false};

  std::size_t idx = 0;
  for (auto bit : bitset) {
    bit = (idx % 2 == 0);
    ++idx;
  }
  for (std::size_t i = 0; i < size; ++i) {
    THES_ALWAYS_ASSERT(bitset.get(i) == (i % 2 == 0));
  }

  std::ranges::fill(bitset, true);
  THES_ALWAYS_ASSERT(std::ranges::all_of(std::as_const(bitset), [](bool v) { return v; }));
}

//--------------------------------------------------------------------------------------------------
// `countr_zero` and `countr_one`, checked against a naive scan over the range interface
//--------------------------------------------------------------------------------------------------

template<typename Bitset>
void test_countr(const Bitset& bitset) {
  const auto naive = [&](bool value) {
    std::size_t count = 0;
    for (const bool bit : bitset) {
      if (bit != value) {
        break;
      }
      ++count;
    }
    return count;
  };
  THES_ALWAYS_ASSERT(bitset.countr_zero() == naive(false));
  THES_ALWAYS_ASSERT(bitset.countr_one() == naive(true));
}

//--------------------------------------------------------------------------------------------------
// Sizes around chunk boundaries, including the empty bitset
//--------------------------------------------------------------------------------------------------

template<std::size_t ChunkByteNum>
void test_edge_cases() {
  constexpr std::size_t chunk_bit_num = thes::FixedBitset<ChunkByteNum>::chunk_bit_num;
  for (const std::size_t size : {std::size_t{0}, std::size_t{1}, chunk_bit_num - 1, chunk_bit_num,
                                 chunk_bit_num + 1, 3 * chunk_bit_num}) {
    test_set_unset_get<ChunkByteNum>(size);
    test_fill<ChunkByteNum>(size);
    test_mutable_range<ChunkByteNum>(size);
  }
}
} // namespace

int main() {
  check_range_concepts<thes::FixedBitset<1>>();
  check_range_concepts<thes::FixedBitset<2>>();
  check_range_concepts<thes::FixedBitset<4>>();
  check_range_concepts<thes::FixedBitset<8>>();

  test_edge_cases<1>();
  test_edge_cases<2>();
  test_edge_cases<4>();
  test_edge_cases<8>();

  // A hand-checked scenario constructed from an explicit sequence of bits, kept as a fixed
  // regression test in addition to the randomized coverage above.
  thes::FixedBitset<4> bitset{
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
    fmt::print("{}\n", bitset);
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
    fmt::print("@{}: {}\n", idx, v1);
  };

  assert_eq();

  set(2);
  unset(bitset.size() - 1);
  get(1);
  test_countr(bitset);

  unset(1);
  test_countr(bitset);

  fmt::print("All fixed-bitset tests passed.\n");
}
