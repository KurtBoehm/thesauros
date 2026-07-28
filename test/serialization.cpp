// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <array>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <vector>

#include "thesauros/containers/array/typed-chunk.hpp"
#include "thesauros/containers/nested-dynamic-array.hpp"
#include "thesauros/filesystem/tempfile.hpp"
#include "thesauros/io/file-reader.hpp"
#include "thesauros/io/file-writer.hpp"
#include "thesauros/io/serialization.hpp"
#include "thesauros/test/equality.hpp"
#include "thesauros/test/test.hpp"
#include "thesauros/types/type-tag.hpp"

namespace {
using Chunk = thes::array::TypedChunk<int, std::size_t, std::allocator<int>>;
using Nested = thes::NestedDynamicArray<int, std::size_t>;

/** Writes `value` to a fresh file under `dir` and reads it back as a `T`. */
template<typename T>
T round_trip(const std::filesystem::path& dir, const T& value) {
  const auto path = dir / "data.bin";
  {
    thes::FileWriter writer{path};
    thes::to_file(value, writer);
  }
  thes::FileReader reader{path};
  return thes::from_file(reader, thes::type_tag<T>);
}

/** Collects the elements of a `TypedChunk` into a `std::vector` for comparison. */
std::vector<int> to_vector(const Chunk& chunk) {
  const auto span = chunk.span();
  return std::vector<int>(span.begin(), span.end());
}

THES_TEST_CASE("TypedChunk round-trips through a file", "[io][serialization]") {
  const thes::fs::TemporaryDirectory dir{};

  Chunk chunk{4};
  const std::array<int, 4> expected{3, 1, 4, 1};
  std::ranges::copy(expected, chunk.span().begin());

  const auto restored = round_trip(dir.path(), chunk);
  THES_CHECK(restored.size() == 4);
  THES_CHECK(thes::test::range_eq(to_vector(restored), expected));
}

THES_TEST_CASE("An empty TypedChunk round-trips", "[io][serialization]") {
  const thes::fs::TemporaryDirectory dir{};

  const Chunk chunk{0};
  const auto restored = round_trip(dir.path(), chunk);
  THES_CHECK(restored.size() == 0);
}

THES_TEST_CASE("NestedDynamicArray round-trips through a file", "[io][serialization]") {
  const thes::fs::TemporaryDirectory dir{};

  Nested::NestedBuilder builder{};
  builder.initialize(3, 5);
  {
    auto part = builder.part_builder(0, 0);
    part.emplace(1);
    part.emplace(2);
    part.advance_group();
    part.advance_group();
    part.emplace(3);
    part.emplace(4);
    part.emplace(5);
    part.advance_group();
  }
  const Nested array = builder.build();

  const auto restored = round_trip(dir.path(), array);
  THES_REQUIRE(restored.size() == 3);
  THES_CHECK(restored.element_num() == 5);
  THES_CHECK(thes::test::range_eq(restored[0], std::array{1, 2}));
  THES_CHECK(restored[1].empty());
  THES_CHECK(thes::test::range_eq(restored[2], std::array{3, 4, 5}));
}
} // namespace

THES_TEST_MAIN()
