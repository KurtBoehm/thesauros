// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <array>
#include <cstddef>
#include <filesystem>
#include <iterator>
#include <span>
#include <string>
#include <vector>

#include "thesauros/containers/dynamic-buffer.hpp"
#include "thesauros/filesystem/tempfile.hpp"
#include "thesauros/io/delimiter.hpp"
#include "thesauros/io/file-reader.hpp"
#include "thesauros/io/file-writer.hpp"
#include "thesauros/io/file.hpp"
#include "thesauros/test/equality.hpp"
#include "thesauros/test/test.hpp"
#include "thesauros/types/primitives.hpp"
#include "thesauros/types/type-tag.hpp"

namespace test = thes::test;

namespace {
//==================================================================================================
// The BufferLike concept
//==================================================================================================

static_assert(thes::BufferLike<thes::DynamicBuffer>);
static_assert(thes::BufferLike<std::vector<std::byte>>);
static_assert(thes::BufferLike<std::vector<unsigned char>>);
static_assert(thes::BufferLike<std::string>);
// A buffer of a non-byte-like element type is not a raw byte buffer.
static_assert(!thes::BufferLike<std::vector<thes::u32>>);
static_assert(!thes::BufferLike<int>);

static_assert(thes::ByteLike<std::byte>);
static_assert(thes::ByteLike<char>);
static_assert(thes::ByteLike<unsigned char>);
static_assert(thes::ByteLike<signed char>);

static_assert(int(thes::Seek::set) == SEEK_SET);
static_assert(int(thes::Seek::cur) == SEEK_CUR);
static_assert(int(thes::Seek::end) == SEEK_END);

//==================================================================================================
// Round-tripping through a file
//==================================================================================================

/** Writes `values` to a fresh file below `dir` and returns its path. */
template<typename T>
[[nodiscard]] std::filesystem::path write_file(const thes::fs::TemporaryDirectory& dir,
                                               std::string_view name,
                                               const std::vector<T>& values) {
  const auto path = dir.path() / name;
  thes::FileWriter writer{path};
  writer.write(std::span{values.data(), values.size()});
  return path;
}

/** Checks that values written with `FileWriter` come back out of `FileReader` unchanged. */
THES_TEST_CASE("values round-trip through a file", "[io][file]") {
  const thes::fs::TemporaryDirectory dir{};
  const std::vector<thes::u32> values{1, 2, 3, 4, 5};
  const auto path = write_file(dir, "values.bin", values);

  THES_REQUIRE(std::filesystem::exists(path));
  THES_CHECK(std::filesystem::file_size(path) == values.size() * sizeof(thes::u32));

  thes::FileReader reader{path};
  THES_CHECK(reader.size() == values.size() * sizeof(thes::u32));

  std::vector<thes::u32> read(values.size());
  reader.read(std::span{read.data(), read.size()});
  THES_CHECK(test::range_eq(read, values));
}

/** Checks the single-value overloads of `write` and `read`. */
THES_TEST_CASE("single values round-trip", "[io][file]") {
  const thes::fs::TemporaryDirectory dir{};
  const auto path = dir.path() / "single.bin";

  {
    thes::FileWriter writer{path};
    writer.write(thes::u32{0xDEADBEEF});
    writer.write(thes::i16{-7});
    THES_CHECK(writer.path() == path);
  }

  thes::FileReader reader{path};
  THES_CHECK(reader.read(thes::type_tag<thes::u32>) == 0xDEADBEEF);
  THES_CHECK(reader.read(thes::type_tag<thes::i16>) == -7);
}

/** Checks reading into a `std::array`, whose overload deduces the element count. */
THES_TEST_CASE("arrays are read whole", "[io][file]") {
  const thes::fs::TemporaryDirectory dir{};
  const std::vector<thes::u8> values{9, 8, 7, 6};
  const auto path = write_file(dir, "array.bin", values);

  thes::FileReader reader{path};
  std::array<thes::u8, 4> read{};
  reader.read(read);
  THES_CHECK(test::range_eq(read, values));
}

//==================================================================================================
// Positioning
//==================================================================================================

/** Checks `seek` and `tell` on both the writer and the reader. */
THES_TEST_CASE("seek and tell move the file position", "[io][file]") {
  const thes::fs::TemporaryDirectory dir{};
  const std::vector<thes::u8> values{0, 1, 2, 3, 4, 5, 6, 7};
  const auto path = write_file(dir, "seek.bin", values);

  thes::FileReader reader{path};
  THES_CHECK(reader.tell() == 0);

  reader.seek(4, thes::Seek::set);
  THES_CHECK(reader.tell() == 4);
  THES_CHECK(reader.read(thes::type_tag<thes::u8>) == 4);
  THES_CHECK(reader.tell() == 5);

  reader.seek(-2, thes::Seek::cur);
  THES_CHECK(reader.tell() == 3);
  THES_CHECK(reader.read(thes::type_tag<thes::u8>) == 3);

  reader.seek(0, thes::Seek::end);
  THES_CHECK(reader.tell() == 8);

  // `size` restores the previous position, so it can be called at any point.
  reader.seek(2, thes::Seek::set);
  THES_CHECK(reader.size() == 8);
  THES_CHECK(reader.tell() == 2);
}

/** Checks that `pread` reads at an absolute offset and leaves the position untouched. */
THES_TEST_CASE("pread does not move the file position", "[io][file]") {
  const thes::fs::TemporaryDirectory dir{};
  const std::vector<thes::u8> values{10, 11, 12, 13, 14, 15};
  const auto path = write_file(dir, "pread.bin", values);

  thes::FileReader reader{path};
  reader.seek(1, thes::Seek::set);

  THES_CHECK(reader.pread(3, thes::type_tag<thes::u8>) == 13);
  THES_CHECK(reader.tell() == 1);

  std::array<thes::u8, 2> read{};
  reader.pread(std::span{read.data(), read.size()}, 4);
  THES_CHECK(test::range_eq(read, std::vector<thes::u8>{14, 15}));
  THES_CHECK(reader.tell() == 1);

  // Reading into a buffer resizes it to the requested length.
  thes::DynamicBuffer buffer{};
  reader.pread(buffer, 2, 2);
  THES_CHECK(buffer.size() == 2);
  THES_CHECK(buffer[0] == std::byte{12});
  THES_CHECK(buffer[1] == std::byte{13});

  // A short `try_pread` near the end reports how much it actually read.
  THES_CHECK(reader.try_pread(buffer, 10, 4) == 2);
  THES_CHECK(buffer.size() == 2);
}

//==================================================================================================
// Whole-file reads
//==================================================================================================

/** Checks `read_full` and the free `read_file` for every supported buffer type. */
THES_TEST_CASE("whole files are read into buffers", "[io][file]") {
  const thes::fs::TemporaryDirectory dir{};
  const std::vector<thes::u8> values{1, 2, 3, 4, 5, 6, 7};
  const auto path = write_file(dir, "full.bin", values);

  {
    thes::FileReader reader{path};
    thes::DynamicBuffer buffer{};
    reader.read_full(buffer);
    THES_CHECK(buffer.size() == values.size());
    THES_CHECK(buffer[0] == std::byte{1});
    THES_CHECK(buffer[6] == std::byte{7});
  }
  {
    thes::FileReader reader{path};
    const auto bytes = reader.read_full(thes::type_tag<std::vector<std::byte>>);
    THES_CHECK(bytes.size() == values.size());
  }
  {
    const auto text = thes::read_file<std::string>(path);
    THES_CHECK(text.size() == values.size());
    THES_CHECK(text[0] == '\1');
  }
  {
    std::vector<unsigned char> bytes{};
    thes::read_file(bytes, path);
    THES_CHECK(test::range_eq(bytes, values));
  }
}

/** Checks that an empty file reads back as an empty buffer. */
THES_TEST_CASE("an empty file yields an empty buffer", "[io][file]") {
  const thes::fs::TemporaryDirectory dir{};
  const auto path = dir.path() / "empty.bin";
  {
    const thes::FileWriter writer{path};
  }

  thes::FileReader reader{path};
  THES_CHECK(reader.size() == 0);

  const auto bytes = reader.read_full(thes::type_tag<std::vector<std::byte>>);
  THES_CHECK(bytes.empty());
}

//==================================================================================================
// Error reporting
//==================================================================================================

/** Checks that the failure modes all report a `FileException`. */
THES_TEST_CASE("failures throw FileException", "[io][file]") {
  const thes::fs::TemporaryDirectory dir{};
  const std::vector<thes::u8> values{1, 2, 3};
  const auto path = write_file(dir, "short.bin", values);

  THES_CHECK_THROWS_AS(thes::FileReader{dir.path() / "missing.bin"}, thes::FileException);

  {
    // Asking for more than the file holds is an error, unlike `try_read`.
    thes::FileReader reader{path};
    std::array<thes::u8, 8> read{};
    THES_CHECK_THROWS_AS(reader.read(std::span{read.data(), read.size()}), thes::FileException);
  }
  {
    // `read_full` insists on starting at the beginning of the file.
    thes::FileReader reader{path};
    reader.seek(1, thes::Seek::set);
    thes::DynamicBuffer buffer{};
    THES_CHECK_THROWS_AS(reader.read_full(buffer), thes::FileException);
  }
  {
    // `pread` insists on a full read just like `read`.
    thes::FileReader reader{path};
    std::array<thes::u8, 8> read{};
    THES_CHECK_THROWS_AS(reader.pread(std::span{read.data(), read.size()}, 0), thes::FileException);
  }
}

/** Checks that `try_read` reports a short read at the end of the file instead of throwing. */
THES_TEST_CASE("try_read reports short reads", "[io][file]") {
  const thes::fs::TemporaryDirectory dir{};
  const std::vector<thes::u8> values{1, 2, 3};
  const auto path = write_file(dir, "try.bin", values);

  thes::FileReader reader{path};
  THES_CHECK(!reader.end_of_file());

  std::array<thes::u8, 8> read{};
  THES_CHECK(reader.try_read(std::span{read.data(), read.size()}) == 3);
  THES_CHECK(reader.end_of_file());
  THES_CHECK(reader.handle() != nullptr);
}

//==================================================================================================
// Delimiter
//==================================================================================================

/**
 * Checks that a `Delimiter` emits nothing before the first element and its separator before each
 * later one.
 */
THES_TEST_CASE("a delimiter separates but does not surround", "[io][delimiter]") {
  std::string out{};
  auto it = std::back_inserter(out);

  thes::Delimiter delim{", "};
  for (int value : {1, 2, 3}) {
    it = delim.output(it);
    *it++ = static_cast<char>('0' + value);
  }
  THES_CHECK(out == "1, 2, 3");
}

/**
 * Checks the same sequence through a plain pointer, where every write has to land at the position
 * the previous call returned rather than simply being appended to a sink.
 */
THES_TEST_CASE("a delimiter threads a positional iterator", "[io][delimiter]") {
  std::array<char, 16> buffer{};
  char* it = buffer.data();

  thes::Delimiter delim{", "};
  for (int value : {1, 2, 3}) {
    it = delim.output(it);
    *it++ = static_cast<char>('0' + value);
  }
  THES_CHECK(std::string_view(buffer.data(), it) == "1, 2, 3");
}

/** Checks the two-argument overload through a plain pointer, for the same reason. */
THES_TEST_CASE("a trailing character is written after the separator", "[io][delimiter]") {
  std::array<char, 16> buffer{};
  char* it = buffer.data();

  thes::Delimiter delim{","};
  for (int value : {1, 2, 3}) {
    it = delim.output(it, '\n');
    *it++ = static_cast<char>('0' + value);
  }
  THES_CHECK(std::string_view(buffer.data(), it) == "1,\n2,\n3");
}

/** Checks that a delimiter can be used in a constant expression. */
THES_TEST_CASE("a delimiter works at compile time", "[io][delimiter]") {
  static constexpr auto joined = [] {
    std::array<char, 8> buffer{};
    char* it = buffer.data();
    thes::Delimiter delim{"-"};
    for (char c : {'a', 'b', 'c'}) {
      it = delim.output(it);
      *it++ = c;
    }
    return buffer;
  }();
  static_assert(std::string_view{joined.data(), 5} == "a-b-c");

  THES_CHECK(std::string_view(joined.data(), 5) == "a-b-c");
}

/** Checks that a fresh delimiter starts over, and that a single element gets no separator. */
THES_TEST_CASE("a delimiter starts over for each instance", "[io][delimiter]") {
  {
    std::string out{};
    auto it = std::back_inserter(out);
    thes::Delimiter delim{"|"};
    it = delim.output(it);
    THES_CHECK(out.empty());
    it = delim.output(it);
    THES_CHECK(out == "|");
    it = delim.output(it);
    THES_CHECK(out == "||");
  }
  {
    // An empty separator still tracks the first call, it just emits nothing.
    std::string out{};
    auto it = std::back_inserter(out);
    thes::Delimiter delim{""};
    it = delim.output(it);
    it = delim.output(it);
    THES_CHECK(out.empty());
  }
}

/** Checks the overload that appends an extra character after the separator. */
THES_TEST_CASE("a delimiter can append a trailing character", "[io][delimiter]") {
  std::string out{};
  auto it = std::back_inserter(out);

  thes::Delimiter delim{","};
  it = delim.output(it, '\n');
  THES_CHECK(out.empty());

  it = delim.output(it, '\n');
  THES_CHECK(out == ",\n");

  it = delim.output(it, '\n');
  THES_CHECK(out == ",\n,\n");
}
} // namespace

THES_TEST_MAIN()
