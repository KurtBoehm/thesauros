// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <atomic>
#include <concepts>
#include <cstddef>
#include <limits>

#include "thesauros/test/test.hpp"
#include "thesauros/types/primitives.hpp"
#include "thesauros/utility/multi-bit-reference.hpp"

namespace {
template<std::size_t ChunkSize>
using Ref = thes::MutableBitReference<ChunkSize>;

static_assert(Ref<1>::chunk_size == 1);
static_assert(std::same_as<Ref<1>::Chunk, thes::u8>);
static_assert(std::same_as<Ref<4>::Chunk, thes::u32>);
static_assert(std::same_as<Ref<8>::Chunk, thes::u64>);

//==================================================================================================
// Reading and writing
//==================================================================================================

/** Checks that a reference reads the addressed bit and nothing else. */
THES_TEST_CASE("a bit reference reads its own bit", "[utility][multi-bit-reference]") {
  thes::u8 chunk{0b1010'0101};

  for (std::size_t i = 0; i < 8; ++i) {
    const bool expected = ((chunk >> i) & 1U) != 0;
    THES_CHECK(bool(Ref<1>{chunk, i}) == expected);
  }
}

/** Checks that assigning through a reference sets or clears exactly one bit. */
THES_TEMPLATE_TEST_CASE("assignment affects a single bit", "[utility][multi-bit-reference]",
                        thes::u8, thes::u16, thes::u32, thes::u64) {
  static constexpr std::size_t digits = std::numeric_limits<TestType>::digits;
  using BitRef = thes::MutableBitReference<sizeof(TestType)>;

  TestType chunk{};
  for (std::size_t i = 0; i < digits; ++i) {
    BitRef{chunk, i} = true;
    THES_CHECK(chunk == static_cast<TestType>(TestType{1} << i));

    BitRef{chunk, i} = false;
    THES_CHECK(chunk == 0);
  }

  // Setting every bit in turn accumulates into an all-ones chunk.
  for (std::size_t i = 0; i < digits; ++i) {
    BitRef{chunk, i} = true;
  }
  THES_CHECK(chunk == std::numeric_limits<TestType>::max());

  for (std::size_t i = 0; i < digits; ++i) {
    BitRef{chunk, i} = false;
  }
  THES_CHECK(chunk == 0);
}

/** Checks that assignment is idempotent, so writing the current value changes nothing. */
THES_TEST_CASE("assignment is idempotent", "[utility][multi-bit-reference]") {
  thes::u16 chunk{0b0011'1100'0011'1100};
  const thes::u16 before = chunk;

  for (std::size_t i = 0; i < 16; ++i) {
    const Ref<2> ref{chunk, i};
    ref = bool(ref);
  }
  THES_CHECK(chunk == before);
}

//==================================================================================================
// Compound assignment
//==================================================================================================

/** Checks that `|=` only ever sets bits and `&=` only ever clears them. */
THES_TEST_CASE("compound assignment is one-directional", "[utility][multi-bit-reference]") {
  {
    thes::u8 chunk{0b0000'0001};
    Ref<1>{chunk, 0} |= false;
    THES_CHECK(chunk == 0b0000'0001);
    Ref<1>{chunk, 1} |= false;
    THES_CHECK(chunk == 0b0000'0001);
    Ref<1>{chunk, 1} |= true;
    THES_CHECK(chunk == 0b0000'0011);
    Ref<1>{chunk, 1} |= true;
    THES_CHECK(chunk == 0b0000'0011);
  }
  {
    thes::u8 chunk{0b0000'0011};
    Ref<1>{chunk, 0} &= true;
    THES_CHECK(chunk == 0b0000'0011);
    Ref<1>{chunk, 2} &= true;
    THES_CHECK(chunk == 0b0000'0011);
    Ref<1>{chunk, 1} &= false;
    THES_CHECK(chunk == 0b0000'0001);
    Ref<1>{chunk, 1} &= false;
    THES_CHECK(chunk == 0b0000'0001);
  }
}

/** Checks that the operators return a reference, so that they chain. */
THES_TEST_CASE("the operators chain", "[utility][multi-bit-reference]") {
  thes::u8 chunk{};
  const Ref<1> ref{chunk, 3};

  (ref = true) |= false;
  THES_CHECK(chunk == 0b0000'1000);

  (ref = false) |= true;
  THES_CHECK(chunk == 0b0000'1000);
}

//==================================================================================================
// Atomic access
//==================================================================================================

/** Checks that `store` writes the same bit as plain assignment. */
THES_TEST_CASE("store matches plain assignment", "[utility][multi-bit-reference]") {
  thes::u32 atomic{};
  thes::u32 plain{};

  for (std::size_t i = 0; i < 32; ++i) {
    const bool value = (i % 3) == 0;
    Ref<4>{atomic, i}.store(value, std::memory_order_relaxed);
    Ref<4>{plain, i} = value;
  }
  THES_CHECK(atomic == plain);

  // Clearing works the same way.
  for (std::size_t i = 0; i < 32; ++i) {
    Ref<4>{atomic, i}.store(false, std::memory_order_relaxed);
  }
  THES_CHECK(atomic == 0);
}
} // namespace

THES_TEST_MAIN()
