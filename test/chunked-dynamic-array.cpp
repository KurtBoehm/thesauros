// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <array>
#include <cstddef>
#include <iterator>

#include "thesauros/containers.hpp"
#include "thesauros/format.hpp"
#include "thesauros/memory/huge-pages-allocator.hpp"
#include "thesauros/ranges.hpp"
#include "thesauros/test.hpp"

namespace test = thes::test;

struct S {
  S() = default;
  explicit S(int j) : i(j) {}

  S(const S&) = default;
  S(S&&) = default;
  S& operator=(const S&) = default;
  S& operator=(S&&) = default;

  ~S() {
    ++counter();
  }

  int i{2};

  static int& counter() {
    static int ctr{0};
    return ctr;
  }

  bool operator==(const S&) const = default;
};

template<>
struct fmt::formatter<S> : fmt::nested_formatter<int> {
  auto format(S s, format_context& ctx) const {
    return this->write_padded(ctx, [&](auto out) { return fmt::format_to(out, "S({})", s.i); });
  }
};

namespace {
using Chunked = thes::ChunkedDynamicArray<S>;
using Nested = thes::NestedDynamicArray<S, std::size_t>;
using NestedBuilder = Nested::NestedBuilder;

//==================================================================================================
// Sizing, growth and destruction
//==================================================================================================

/** Checks block sizes after `add_blocks()` and `emplace_back()`, via `std::views::transform`. */
THES_TEST_CASE("multi block sizes", "[chunked]") {
  thes::ChunkedDynamicArray<int> arr{8};
  arr.add_blocks(4);
  arr[0].emplace_back(2);
  arr[0].emplace_back(5);
  arr[3].emplace_back(3);

  THES_CHECK(test::range_eq(arr | std::views::transform([](auto block) { return block.size(); }),
                            std::array<std::size_t, 4>{2, 0, 0, 1}));
}

/**
 * Checks `block_num()`, `value_num()` and `alloc_size()` on an empty container, that `begin()`
 * equals `end()` when empty, and that `pop_block()` shrinks the container correctly.
 */
THES_TEST_CASE("empty container and pop_block", "[chunked]") {
  thes::ChunkedDynamicArray<int> arr{8};
  THES_REQUIRE(arr.block_num() == 0);
  THES_REQUIRE(arr.value_num() == 0);
  THES_CHECK(arr.alloc_size() == 0);
  THES_CHECK(arr.begin() == arr.end());

  arr.push_block();
  arr.push_block();
  arr[0].emplace_back(1);
  arr[0].emplace_back(2);
  arr[1].emplace_back(3);
  THES_REQUIRE(arr.block_num() == 2);
  THES_REQUIRE(arr.value_num() == 3);

  arr.pop_block();
  THES_REQUIRE(arr.block_num() == 1);
  THES_REQUIRE(arr.value_num() == 2);

  arr.pop_block();
  THES_CHECK(arr.block_num() == 0);
  THES_REQUIRE(arr.value_num() == 0);
}

/** Checks that `add_blocks(0)` is a no-op that skips reallocation entirely. */
THES_TEST_CASE("add_blocks(0) is a no-op", "[chunked]") {
  thes::ChunkedDynamicArray<int> arr{4};
  arr.add_blocks(0);
  THES_REQUIRE(arr.block_num() == 0);
  arr.push_block();
  arr.add_blocks(0);
  THES_CHECK(arr.block_num() == 1);
}

/**
 * Exercises `push_block()`, growth and reallocation across many `push_block()` calls (triggering
 * `DoublingGrowth` reallocation), and checks that reallocation preserves existing block contents
 * and destroys the old storage exactly once per live element, tracked via `S::counter()`.
 */
THES_TEST_CASE("growth and reallocation", "[chunked]") {
  Chunked vec{8};
  THES_REQUIRE(vec.block_num() == 0);
  THES_REQUIRE(vec.value_num() == 0);

  S::counter() = 0;
  vec.push_block();

  {
    auto block1 = *(vec.end() - 1);
    THES_REQUIRE(vec.block_num() == 1);
    THES_REQUIRE(vec.value_num() == 0);
    THES_REQUIRE(block1.size() == 0);

    block1.emplace_back(3);
    THES_REQUIRE(vec.block_num() == 1);
    THES_REQUIRE(vec.value_num() == 1);
    THES_REQUIRE(test::range_eq(block1, std::array{S{3}}));

    for (const auto i : thes::views::indices<int>(1, 8)) {
      block1.emplace_back(i);
    }
    THES_REQUIRE(vec.block_num() == 1);
    THES_REQUIRE(vec.value_num() == 8);
    THES_REQUIRE(
      test::range_eq(block1, std::array{S{3}, S{1}, S{2}, S{3}, S{4}, S{5}, S{6}, S{7}}));
  }

  vec.push_block();
  auto block1 = vec[0];
  auto block2 = *(vec.end() - 1);
  THES_REQUIRE(S::counter() == 9);
  THES_REQUIRE(vec.block_num() == 2);
  THES_REQUIRE(vec.value_num() == 8);
  THES_REQUIRE(test::range_eq(block1, std::array{S{3}, S{1}, S{2}, S{3}, S{4}, S{5}, S{6}, S{7}}));
  THES_REQUIRE(block2.size() == 0);

  block2.emplace_back(2);
  block2.emplace_back(3);
  block2.emplace_back(5);
  block2.emplace_back(11);
  block2.emplace_back(17);
  THES_REQUIRE(vec.block_num() == 2);
  THES_REQUIRE(vec.value_num() == 13);
  THES_REQUIRE(test::range_eq(block1, std::array{S{3}, S{1}, S{2}, S{3}, S{4}, S{5}, S{6}, S{7}}));
  THES_REQUIRE(test::range_eq(block2, std::array{S{2}, S{3}, S{5}, S{11}, S{17}}));

  block1.erase(S{2});
  block1.erase(S{3});
  THES_REQUIRE(vec.block_num() == 2);
  THES_REQUIRE(vec.value_num() == 10);
  THES_REQUIRE(test::range_eq(block1, std::array{S{1}, S{4}, S{5}, S{6}, S{7}}));
  THES_REQUIRE(test::range_eq(block2, std::array{S{2}, S{3}, S{5}, S{11}, S{17}}));

  S::counter() = 0;
  for ([[maybe_unused]] const auto i : thes::views::indices<std::size_t>(30)) {
    vec.push_block();
    vec.push_block();
  }
  THES_REQUIRE(S::counter() == 50);

  {
    auto b1 = vec[0];
    auto b2 = vec[1];
    THES_CHECK(vec.block_num() == 62);
    THES_REQUIRE(vec.value_num() == 10);
    THES_CHECK(test::range_eq(b1, std::array{S{1}, S{4}, S{5}, S{6}, S{7}}));
    THES_CHECK(test::range_eq(b2, std::array{S{2}, S{3}, S{5}, S{11}, S{17}}));
  }
}

/**
 * Checks that `pop_block()` destroys exactly the live elements of the popped block, rather than
 * its whole capacity.
 */
THES_TEST_CASE("pop_block destructors", "[chunked]") {
  S::counter() = 0;
  thes::ChunkedDynamicArray<S> arr{4};
  arr.push_block();
  arr[0].emplace_back(1);
  arr[0].emplace_back(2);
  arr[0].emplace_back(3);
  THES_REQUIRE(S::counter() == 0);

  arr.pop_block();
  THES_CHECK(S::counter() == 3);
  THES_CHECK(arr.block_num() == 0);
}

//==================================================================================================
// Block element access
//==================================================================================================

/** Checks `MutableBlock::push_back()`, both the copy and the move overload. */
THES_TEST_CASE("push_back copy and move", "[chunked][block]") {
  thes::ChunkedDynamicArray<S> arr{4};
  arr.push_block();
  auto block = arr[0];

  S lvalue{7};
  block.push_back(lvalue);
  block.push_back(S{9});
  THES_REQUIRE(block.size() == 2);
  THES_CHECK(test::range_eq(block, std::array{S{7}, S{9}}));
}

/**
 * Checks `Block::operator[]`, `Block::span()`, and `ChunkedDynamicArrayBase::operator[] const`
 * returning a `ConstBlock`.
 */
THES_TEST_CASE("block indexing and span", "[chunked][block]") {
  thes::ChunkedDynamicArray<int> arr{4};
  arr.push_block();
  auto block = arr[0];
  block.emplace_back(10);
  block.emplace_back(20);
  block.emplace_back(30);

  THES_CHECK(block[0] == 10);
  THES_REQUIRE(block[1] == 20);
  THES_REQUIRE(block[2] == 30);

  auto sp = block.span();
  THES_REQUIRE(sp.size() == 3);
  THES_CHECK(sp[0] == 10);
  THES_REQUIRE(sp[2] == 30);

  const auto& carr = arr;
  auto cblock = carr[0];
  THES_CHECK(test::range_eq(cblock, std::array{10, 20, 30}));
}

/**
 * Checks `erase()` for a value not present, which should be a no-op, and repeated `erase()`
 * calls that empty the block entirely.
 */
THES_TEST_CASE("erase all and missing", "[chunked][block]") {
  thes::ChunkedDynamicArray<int> arr{4};
  arr.push_block();
  auto block = arr[0];
  block.emplace_back(1);
  block.emplace_back(2);
  block.emplace_back(3);

  block.erase(42);
  THES_REQUIRE(block.size() == 3);

  block.erase(1);
  block.erase(2);
  block.erase(3);
  THES_CHECK(block.size() == 0);
}

//==================================================================================================
// Iteration
//==================================================================================================

/** Checks const `begin()`/`end()` and iteration over `ConstBlock`. */
THES_TEST_CASE("const iteration", "[chunked][iterator]") {
  thes::ChunkedDynamicArray<int> arr{4};
  arr.add_blocks(3);
  arr[0].emplace_back(1);
  arr[1].emplace_back(2);
  arr[1].emplace_back(3);

  const auto& carr = arr;
  std::size_t total = 0;
  for (const auto block : carr) {
    total += block.size();
  }
  THES_CHECK(total == 3);
  THES_CHECK(carr.begin() != carr.end());
}

/**
 * Checks random-access iterator arithmetic and comparisons: `operator+`/`operator-`,
 * `operator+=`/`operator-=`, prefix and postfix `++`/`--`, `operator[]`, the difference between
 * two iterators, the full set of relational operators, and compatibility with `std::distance`,
 * `std::next` and `std::prev`.
 */
THES_TEST_CASE("random access iterator", "[chunked][iterator]") {
  using Array = thes::ChunkedDynamicArray<int>;

  static_assert(std::ranges::random_access_range<Array>);
  static_assert(std::random_access_iterator<Array::const_iterator>);
  static_assert(std::random_access_iterator<Array::iterator>);

  thes::ChunkedDynamicArray<int> arr{4};
  arr.add_blocks(5);
  for (const auto i : thes::views::indices<std::size_t>(5)) {
    arr[i].emplace_back(static_cast<int>(i));
  }

  auto begin = arr.begin();
  auto end = arr.end();

  THES_REQUIRE(end - begin == 5);
  THES_CHECK(begin - end == -5);

  auto it = begin + 2;
  THES_REQUIRE((*it)[0] == 2);
  THES_CHECK(it - begin == 2);
  THES_CHECK(begin[2][0] == 2);

  it += 2;
  THES_REQUIRE((*it)[0] == 4);
  it -= 3;
  THES_REQUIRE((*it)[0] == 1);

  auto pre = ++it;
  THES_REQUIRE((*it)[0] == 2);
  THES_REQUIRE((*pre)[0] == 2);
  auto post = it++;
  THES_REQUIRE((*post)[0] == 2);
  THES_REQUIRE((*it)[0] == 3);

  auto pre_dec = --it;
  THES_REQUIRE((*it)[0] == 2);
  THES_REQUIRE((*pre_dec)[0] == 2);
  auto post_dec = it--;
  THES_CHECK((*post_dec)[0] == 2);
  THES_REQUIRE((*it)[0] == 1);

  THES_CHECK(begin < end);
  THES_CHECK(end > begin);
  THES_CHECK(begin <= begin);
  THES_CHECK(begin >= begin);
  THES_CHECK(begin != end);
  THES_CHECK(begin + 5 == end);

  THES_CHECK(std::ranges::distance(begin, end) == 5);
  THES_CHECK((*std::ranges::next(begin, 3))[0] == 3);
  THES_CHECK((*std::ranges::prev(end, 1))[0] == 4);
}

//==================================================================================================
// Integration with other components
//==================================================================================================

/**
 * Builds a small `Chunked` vector and feeds it through `NestedBuilder`, checking that the
 * resulting `Nested` container exposes the same group counts and contents.
 */
THES_TEST_CASE("nested builder integration", "[chunked][nested]") {
  Chunked vec{8};
  vec.push_block();
  auto block1 = *(vec.end() - 1);
  for (const auto i : thes::views::indices<int>(0, 8)) {
    block1.emplace_back(i);
  }
  vec.push_block();
  auto b0 = vec[0];
  auto b1 = vec[1];
  b0.erase(S{2});
  b0.erase(S{3});
  b1.emplace_back(2);
  b1.emplace_back(3);
  b1.emplace_back(5);
  b1.emplace_back(11);
  b1.emplace_back(17);

  NestedBuilder builder{};
  builder.initialize(vec.block_num(), vec.value_num());
  {
    auto part = builder.part_builder(0, 0);
    for (auto&& val : vec[0]) {
      part.emplace(val);
    }
    part.advance_group();
  }
  {
    auto part = builder.part_builder(1, vec[0].size());
    for (const std::size_t i : thes::views::indices<std::size_t>(1, vec.block_num())) {
      for (auto&& val : vec[i]) {
        part.emplace(val);
      }
      part.advance_group();
    }
  }

  Nested nested = builder.build();
  THES_CHECK(nested.group_num() == vec.block_num());
  THES_CHECK(nested.element_num() == vec.value_num());
  THES_CHECK(test::range_eq(vec[0], std::array{S{0}, S{1}, S{4}, S{5}, S{6}, S{7}}));
  THES_CHECK(test::range_eq(vec[1], std::array{S{2}, S{3}, S{5}, S{11}, S{17}}));
}

/**
 * Instantiates `ChunkedDynamicArrayBase` directly with `thes::HugePagesAllocator`, covering both
 * the move-allocator and the const-reference-allocator constructors, and checks that reallocation
 * preserves contents when using huge-page-backed storage.
 */
THES_TEST_CASE("custom allocator", "[chunked][allocator]") {
  using HugeChunked =
    thes::ChunkedDynamicArrayBase<int, std::size_t, thes::HugePagesAllocator<int>,
                                  thes::HugePagesAllocator<std::size_t>, thes::DoublingGrowth>;

  {
    HugeChunked arr{8, thes::HugePagesAllocator<int>{}, thes::HugePagesAllocator<std::size_t>{}};
    arr.push_block();
    arr[0].emplace_back(42);
    THES_REQUIRE(arr.block_num() == 1);
    THES_REQUIRE(arr.value_num() == 1);
    THES_CHECK(arr[0][0] == 42);
  }

  {
    const thes::HugePagesAllocator<int> value_alloc{};
    const thes::HugePagesAllocator<std::size_t> size_alloc{};
    HugeChunked arr{8, value_alloc, size_alloc};
    arr.add_blocks(3);
    arr[1].emplace_back(1);
    arr[1].emplace_back(2);
    THES_REQUIRE(arr.block_num() == 3);
    THES_REQUIRE(arr.value_num() == 2);
    THES_REQUIRE(test::range_eq(arr[1], std::array{1, 2}));

    for ([[maybe_unused]] const auto i : thes::views::indices<std::size_t>(20)) {
      arr.push_block();
    }
    THES_CHECK(arr.block_num() == 23);
    THES_REQUIRE(arr.value_num() == 2);
    THES_CHECK(test::range_eq(arr[1], std::array{1, 2}));
  }
}
} // namespace

THES_TEST_MAIN()
