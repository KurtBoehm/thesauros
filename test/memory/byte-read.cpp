// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <array>
#include <bit>
#include <cstddef>
#include <cstring>

#include "thesauros/memory/byte-read.hpp"
#include "thesauros/test/test.hpp"
#include "thesauros/types/primitives.hpp"

namespace {
/** Reinterprets `storage` as a byte pointer, which is what `byte_read` consumes. */
template<typename T>
[[nodiscard]] std::byte* bytes_of(T& storage) {
  return reinterpret_cast<std::byte*>(&storage); // NOLINT(*-reinterpret-cast)
}

//==================================================================================================
// Reading whole objects
//==================================================================================================

/** Checks that `byte_read` reproduces a value written into raw storage. */
THES_TEMPLATE_TEST_CASE("byte_read round-trips trivial values", "[memory][byte-read]", thes::u8,
                        thes::u16, thes::u32, thes::u64, thes::i32, thes::i64, thes::f32,
                        thes::f64) {
  std::array<std::byte, 2 * sizeof(TestType)> storage{};

  const TestType value{42};
  std::memcpy(storage.data(), &value, sizeof(TestType));
  THES_CHECK(thes::byte_read<TestType>(storage.data()) == value);

  // The same value written at an offset is read back from that offset.
  std::memcpy(storage.data() + sizeof(TestType), &value, sizeof(TestType));
  THES_CHECK(thes::byte_read<TestType>(storage.data() + sizeof(TestType)) == value);
}

/** Checks that a trivial aggregate is read back field by field. */
THES_TEST_CASE("byte_read handles trivial aggregates", "[memory][byte-read]") {
  struct Pair {
    thes::u32 first;
    thes::u16 second;
  };
  static_assert(std::is_trivial_v<Pair>);

  Pair source{.first = 0xABCDEF01, .second = 0x2345};
  const auto read = thes::byte_read<Pair>(bytes_of(source));

  THES_CHECK(read.first == source.first);
  THES_CHECK(read.second == source.second);
}

//==================================================================================================
// Unaligned and partial access
//==================================================================================================

/** Checks that reading works at an unaligned address, which is why `std::memcpy` is used. */
THES_TEST_CASE("byte_read works unaligned", "[memory][byte-read]") {
  std::array<std::byte, 16> storage{};
  const thes::u64 value{0x0123456789ABCDEF};

  for (std::size_t offset = 0; offset < 8; ++offset) {
    storage.fill(std::byte{0});
    std::memcpy(storage.data() + offset, &value, sizeof(value));
    THES_CHECK(thes::byte_read<thes::u64>(storage.data() + offset) == value);
  }
}

/** Checks that a narrow read picks out the bytes at its position, per the host’s endianness. */
THES_TEST_CASE("a narrow read sees only its own bytes", "[memory][byte-read]") {
  thes::u32 source = 0x11223344;
  std::byte* const bytes = bytes_of(source);

  const auto low = thes::byte_read<thes::u16>(bytes);
  const auto high = thes::byte_read<thes::u16>(bytes + sizeof(thes::u16));

  if constexpr (std::endian::native == std::endian::little) {
    THES_CHECK(low == 0x3344);
    THES_CHECK(high == 0x1122);
  } else {
    THES_CHECK(low == 0x1122);
    THES_CHECK(high == 0x3344);
  }

  // Either way, recombining the halves reproduces the original value.
  const auto recombined = (std::endian::native == std::endian::little)
                            ? (thes::u32{high} << 16U) | thes::u32{low}
                            : (thes::u32{low} << 16U) | thes::u32{high};
  THES_CHECK(recombined == source);
}

/** Checks that reading from zeroed storage yields a zeroed value. */
THES_TEST_CASE("zeroed storage reads as zero", "[memory][byte-read]") {
  std::array<std::byte, 8> storage{};
  THES_CHECK(thes::byte_read<thes::u64>(storage.data()) == 0);
  THES_CHECK(thes::byte_read<thes::u32>(storage.data()) == 0);
  THES_CHECK(thes::byte_read<thes::f64>(storage.data()) == 0.0);
}
} // namespace

THES_TEST_MAIN()
