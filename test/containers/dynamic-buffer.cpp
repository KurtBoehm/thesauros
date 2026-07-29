// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <span>
#include <string_view>
#include <utility>

#include "thesauros/containers/dynamic-buffer.hpp"
#include "thesauros/test/test.hpp"

namespace {
/** Fills `buffer` with the byte values `0, 1, 2, …`, wrapping around after 256. */
void fill_iota(thes::DynamicBuffer& buffer) {
  for (std::size_t i = 0; i < buffer.size(); ++i) {
    buffer[i] = static_cast<std::byte>(i);
  }
}

/** Returns whether `buffer` still holds the pattern written by `fill_iota` up to `size`. */
[[nodiscard]] bool holds_iota(const thes::DynamicBuffer& buffer, std::size_t size) {
  for (std::size_t i = 0; i < size; ++i) {
    if (buffer[i] != static_cast<std::byte>(i)) {
      return false;
    }
  }
  return true;
}

//==================================================================================================
// Construction
//==================================================================================================

/** Checks that a default-constructed buffer is empty and holds no storage. */
THES_TEST_CASE("a default-constructed buffer is empty", "[containers][dynamic-buffer]") {
  const thes::DynamicBuffer buffer{};
  THES_CHECK(buffer.size() == 0);
  THES_CHECK(buffer.data() == nullptr);
  THES_CHECK(buffer.span().empty());
}

/** Checks that the sized constructor reports the requested size and a writable span. */
THES_TEST_CASE("a sized buffer exposes its storage", "[containers][dynamic-buffer]") {
  thes::DynamicBuffer buffer{16};
  THES_REQUIRE(buffer.size() == 16);
  THES_CHECK(buffer.data() != nullptr);
  THES_CHECK(buffer.span().size() == 16);

  fill_iota(buffer);
  THES_CHECK(holds_iota(buffer, 16));
  THES_CHECK(buffer.span()[3] == std::byte{3});
}

//==================================================================================================
// Resizing
//==================================================================================================

/** Checks that growing a buffer preserves the existing bytes. */
THES_TEST_CASE("growing preserves the contents", "[containers][dynamic-buffer]") {
  thes::DynamicBuffer buffer{8};
  fill_iota(buffer);

  buffer.resize(64);
  THES_CHECK(buffer.size() == 64);
  THES_CHECK(holds_iota(buffer, 8));

  fill_iota(buffer);
  THES_CHECK(holds_iota(buffer, 64));
}

/**
 * Checks that shrinking only lowers the reported size, so that growing back within the previously
 * allocated capacity recovers the bytes beyond the shrunken size.
 */
THES_TEST_CASE("shrinking keeps the allocation", "[containers][dynamic-buffer]") {
  thes::DynamicBuffer buffer{32};
  fill_iota(buffer);

  buffer.resize(8);
  THES_CHECK(buffer.size() == 8);
  THES_CHECK(buffer.span().size() == 8);
  THES_CHECK(holds_iota(buffer, 8));

  buffer.resize(32);
  THES_CHECK(buffer.size() == 32);
  THES_CHECK(holds_iota(buffer, 32));
}

/** Checks that resizing to the current size is a no-op and that resizing to zero is allowed. */
THES_TEST_CASE("resizing to the same size and to zero", "[containers][dynamic-buffer]") {
  thes::DynamicBuffer buffer{4};
  fill_iota(buffer);

  const auto* const before = buffer.data();
  buffer.resize(4);
  THES_CHECK(buffer.size() == 4);
  THES_CHECK(buffer.data() == before);
  THES_CHECK(holds_iota(buffer, 4));

  buffer.resize(0);
  THES_CHECK(buffer.size() == 0);
  THES_CHECK(buffer.span().empty());
}

/** Checks that a default-constructed buffer can be grown from nothing. */
THES_TEST_CASE("an empty buffer can be grown", "[containers][dynamic-buffer]") {
  thes::DynamicBuffer buffer{};
  buffer.resize(24);
  THES_REQUIRE(buffer.size() == 24);
  THES_CHECK(buffer.data() != nullptr);

  fill_iota(buffer);
  THES_CHECK(holds_iota(buffer, 24));
}

//==================================================================================================
// Typed views on the storage
//==================================================================================================

/** Checks that the typed data pointers all alias the same storage. */
THES_TEST_CASE("the typed data pointers alias the same bytes", "[containers][dynamic-buffer]") {
  static constexpr std::string_view text{"thesauros"};

  thes::DynamicBuffer buffer{text.size()};
  std::ranges::copy(text, buffer.data_char());

  const thes::DynamicBuffer& cbuffer = buffer;
  THES_CHECK(std::string_view(cbuffer.data_char(), cbuffer.size()) == text);
  THES_CHECK(cbuffer.data_u8()[0] == static_cast<unsigned char>('t'));
  THES_CHECK(cbuffer[0] == static_cast<std::byte>('t'));
  THES_CHECK(std::memcmp(cbuffer.data(), text.data(), text.size()) == 0);

  buffer[0] = static_cast<std::byte>('T');
  THES_CHECK(buffer.data_char()[0] == 'T');
  THES_CHECK(buffer.data_u8()[0] == static_cast<unsigned char>('T'));
}

//==================================================================================================
// Move semantics
//==================================================================================================

/** Checks that moving transfers the storage and leaves the source empty. */
THES_TEST_CASE("moving transfers the storage", "[containers][dynamic-buffer]") {
  thes::DynamicBuffer source{16};
  fill_iota(source);
  const auto* const data = source.data();

  const thes::DynamicBuffer target{std::move(source)};
  THES_CHECK(target.size() == 16);
  THES_CHECK(target.data() == data);
  THES_CHECK(holds_iota(target, 16));

  THES_CHECK(source.size() == 0);
  THES_CHECK(source.data() == nullptr);
}
} // namespace

THES_TEST_MAIN()
