// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <atomic>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

#include "thesauros/containers/multi-bit-integers.hpp"
#include "thesauros/math/arithmetic.hpp"
#include "thesauros/test/test.hpp"
#include "thesauros/types/primitives.hpp"

namespace {
template<typename Chunk, std::size_t BitNum>
using Mbi = thes::MultiBitIntegers<Chunk, BitNum>;

//==================================================================================================
// Layout
//==================================================================================================

static_assert(Mbi<thes::u32, 4>::per_chunk == 8);
static_assert(Mbi<thes::u32, 4>::mask == 0xF);
static_assert(Mbi<thes::u8, 1>::per_chunk == 8);
static_assert(Mbi<thes::u8, 1>::mask == 1);
static_assert(Mbi<thes::u64, 8>::per_chunk == 8);
static_assert(Mbi<thes::u64, 8>::mask == 0xFF);

/** Checks that the number of chunks is the smallest one that can hold all the elements. */
THES_TEST_CASE("chunk_num covers the requested size", "[containers][multi-bit-integers]") {
  using Bits = Mbi<thes::u32, 4>;
  static constexpr std::size_t per_chunk = Bits::per_chunk;

  for (std::size_t size = 0; size <= 3 * per_chunk; ++size) {
    const Bits bits{size, 0};
    THES_CHECK(bits.size() == size);
    THES_CHECK(bits.chunk_num() == thes::div_ceil(size, per_chunk));
  }
}

//==================================================================================================
// Construction and element access
//==================================================================================================

/** Checks that the filling constructor writes the same value into every element. */
THES_TEMPLATE_TEST_CASE("the filling constructor initializes every element",
                        "[containers][multi-bit-integers]", thes::u8, thes::u16, thes::u32,
                        thes::u64) {
  using Bits = Mbi<TestType, 2>;
  static constexpr TestType mask = Bits::mask;

  for (TestType value = 0; value <= mask; ++value) {
    // A size that is not a multiple of `per_chunk`, so the trailing partial chunk is covered too.
    const Bits bits{Bits::per_chunk + 1, value};
    for (std::size_t i = 0; i < bits.size(); ++i) {
      THES_CHECK(bits[i] == value);
    }
  }
}

/** Checks that assigning through the proxy affects exactly the addressed element. */
THES_TEST_CASE("element assignment is independent", "[containers][multi-bit-integers]") {
  using Bits = Mbi<thes::u32, 4>;
  static constexpr std::size_t size = 20;

  Bits bits{size, 0};
  std::vector<thes::u32> expected(size, 0);

  for (std::size_t i = 0; i < size; ++i) {
    const auto value = static_cast<thes::u32>((3 * i + 1) & Bits::mask);
    bits[i] = value;
    expected[i] = value;
  }

  const Bits& cbits = bits;
  for (std::size_t i = 0; i < size; ++i) {
    THES_CHECK(cbits[i] == expected[i]);
    THES_CHECK(thes::u32{bits[i]} == expected[i]);
  }
}

/** Checks that the maximum representable value can be stored and read back. */
THES_TEST_CASE("the full mask can be stored", "[containers][multi-bit-integers]") {
  using Bits = Mbi<thes::u16, 4>;

  Bits bits{5, 0};
  bits[2] = Bits::mask;
  THES_CHECK(bits[2] == Bits::mask);
  THES_CHECK(bits[1] == thes::u16{0});
  THES_CHECK(bits[3] == thes::u16{0});

  bits[2] = 0;
  THES_CHECK(bits[2] == thes::u16{0});
}

//==================================================================================================
// Bit-level access within an element
//==================================================================================================

/** Checks that `set_bit` and `get_bit` address bits relative to the element, not the chunk. */
THES_TEST_CASE("per-element bit access is element-relative", "[containers][multi-bit-integers]") {
  using Bits = Mbi<thes::u32, 4>;
  static constexpr std::size_t size = 9;

  Bits bits{size, 0};
  for (std::size_t i = 0; i < size; ++i) {
    // Set bit `i % 4` of element `i`, so the resulting value is `1 << (i % 4)`.
    bits[i].set_bit(static_cast<thes::u32>(i % 4), true);
  }
  for (std::size_t i = 0; i < size; ++i) {
    THES_CHECK(bits[i] == thes::u32{1} << (i % 4));
    THES_CHECK(bits[i].get_bit(static_cast<thes::u32>(i % 4)));
  }

  for (std::size_t i = 0; i < size; ++i) {
    bits[i].set_bit(static_cast<thes::u32>(i % 4), false);
    THES_CHECK(!bits[i].get_bit(static_cast<thes::u32>(i % 4)));
    THES_CHECK(bits[i] == thes::u32{0});
  }
}

//==================================================================================================
// Atomic access
//==================================================================================================

/** Checks that the atomic `store`/`load` pair agrees with the plain accessors. */
THES_TEST_CASE("atomic store and load match the plain accessors",
               "[containers][multi-bit-integers]") {
  using Bits = Mbi<thes::u32, 4>;
  static constexpr std::size_t size = 17;

  Bits bits{size, 0};
  for (std::size_t i = 0; i < size; ++i) {
    bits[i].store(static_cast<thes::u32>(i & Bits::mask), std::memory_order_relaxed);
  }

  for (std::size_t i = 0; i < size; ++i) {
    const auto expected = static_cast<thes::u32>(i & Bits::mask);
    THES_CHECK(bits.load(i, std::memory_order_relaxed) == expected);
    THES_CHECK(std::as_const(bits)[i] == expected);
  }
}

/** Checks that the atomic `set_bit` overload agrees with the plain one. */
THES_TEST_CASE("atomic set_bit matches the plain overload", "[containers][multi-bit-integers]") {
  using Bits = Mbi<thes::u64, 8>;
  static constexpr std::size_t size = 10;

  Bits atomic{size, 0};
  Bits plain{size, 0};

  for (std::size_t i = 0; i < size; ++i) {
    for (thes::u64 bit = 0; bit < 8; ++bit) {
      const bool value = ((i + bit) % 3) == 0;
      atomic[i].set_bit(bit, value, std::memory_order_relaxed);
      plain[i].set_bit(bit, value);
    }
  }

  for (std::size_t i = 0; i < size; ++i) {
    THES_CHECK(atomic[i] == plain[i]);
  }
}

//==================================================================================================
// Single-bit elements
//==================================================================================================

/** Checks the degenerate case of one bit per element, where the mask is a single bit. */
THES_TEST_CASE("single-bit elements behave like a bitset", "[containers][multi-bit-integers]") {
  using Bits = Mbi<thes::u8, 1>;
  static constexpr std::size_t size = 19;

  Bits bits{size, 0};
  for (std::size_t i = 0; i < size; ++i) {
    bits[i] = static_cast<thes::u8>(i % 2);
  }
  for (std::size_t i = 0; i < size; ++i) {
    THES_CHECK(bits[i] == thes::u8(i % 2));
  }

  const Bits ones{size, 1};
  for (std::size_t i = 0; i < size; ++i) {
    THES_CHECK(ones[i] == thes::u8{1});
  }
}

/** Checks that elements as wide as the chunk itself round-trip the whole value range. */
THES_TEST_CASE("chunk-wide elements round-trip", "[containers][multi-bit-integers]") {
  using Bits = Mbi<thes::u8, 8>;
  static_assert(Bits::per_chunk == 1);
  static_assert(Bits::mask == std::numeric_limits<thes::u8>::max());

  Bits bits{4, 0};
  bits[0] = 0;
  bits[1] = 1;
  bits[2] = 128;
  bits[3] = 255;

  THES_CHECK(bits.chunk_num() == 4);
  THES_CHECK(bits[0] == thes::u8{0});
  THES_CHECK(bits[1] == thes::u8{1});
  THES_CHECK(bits[2] == thes::u8{128});
  THES_CHECK(bits[3] == thes::u8{255});
}
} // namespace

THES_TEST_MAIN()
