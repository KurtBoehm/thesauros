// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "thesauros/containers.hpp"
#include "thesauros/format.hpp"
#include "thesauros/ranges.hpp"
#include "thesauros/test.hpp"

namespace test = thes::test;

struct S {
  S() {
    fmt::print("S{{}}\n");
  }
  explicit S(int j) : i(j) {
    fmt::print("S{{{}}}\n", i);
  }

  S(const S&) = default;
  S(S&&) = default;
  S& operator=(const S&) = default;
  S& operator=(S&&) = default;

  ~S() {
    fmt::print("~S({})\n", i);
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
struct fmt::formatter<S> : public fmt::nested_formatter<int> {
  auto format(const S& s, format_context& ctx) const {
    return this->write_padded(
      ctx, [&](auto out) { return fmt::format_to(out, "S{{{}}}", nested(s.i)); });
  }
};

namespace {
//==================================================================================================
// Const-correctness of deducing-this accessors
//==================================================================================================

THES_TEST_CASE("DynamicArray: const object yields const_iterator/const_reference",
               "[containers][array][dynamic][const-correctness]") {
  const thes::DynamicArray<int, thes::DefaultInit> darray{1, 2, 3};
  static_assert(std::same_as<decltype(darray.begin()),
                             thes::DynamicArray<int, thes::DefaultInit>::const_iterator>);
  static_assert(std::same_as<decltype(darray.end()),
                             thes::DynamicArray<int, thes::DefaultInit>::const_iterator>);
  static_assert(std::same_as<decltype(darray.front()), const int&>);
  static_assert(std::same_as<decltype(darray.back()), const int&>);
  static_assert(std::same_as<decltype(darray[0]), const int&>);
  THES_CHECK(darray.front() == 1);
  THES_CHECK(darray.back() == 3);
}

THES_TEST_CASE("FixedAllocArray: const object yields const_iterator/const_reference",
               "[containers][array][fixed-alloc][const-correctness]") {
  const thes::FixedAllocArray<int> farray{1, 2, 3};
  static_assert(std::same_as<decltype(farray.begin()), thes::FixedAllocArray<int>::const_iterator>);
  static_assert(std::same_as<decltype(farray.end()), thes::FixedAllocArray<int>::const_iterator>);
  static_assert(
    std::same_as<decltype(farray.rbegin()), thes::FixedAllocArray<int>::const_reverse_iterator>);
  static_assert(std::same_as<decltype(farray.back()), const int&>);
  THES_CHECK(farray.back() == 3);
}

THES_TEST_CASE("LimitedArray: const object yields const_iterator", "[containers][array][limited]") {
  const thes::LimitedArray<int, 4> larray(1, 2, 3);
  static_assert(std::same_as<decltype(larray.begin()), thes::LimitedArray<int, 4>::const_iterator>);
  static_assert(std::same_as<decltype(larray.front()), const int&>);
  THES_CHECK(test::range_eq(larray, std::array{1, 2, 3}));
}

//==================================================================================================
// DynamicArray
//==================================================================================================

THES_TEST_CASE("DynamicArray: value-init construction and resize", "[containers][array][dynamic]") {
  thes::DynamicArray<int, thes::ValueInit> darray1(3);
  THES_REQUIRE(darray1.size() == 3);
  THES_CHECK(!darray1.empty());
  THES_CHECK(test::range_eq(darray1, std::array{0, 0, 0}));

  darray1.resize(9);
  for (const auto i : thes::views::indices<std::size_t>(8)) {
    darray1[i] = static_cast<int>(2 * i + 1);
  }
  THES_REQUIRE(darray1.size() == 9);
  THES_CHECK(darray1.allocation_size() == 16);
  THES_CHECK(test::range_eq(darray1, std::array{1, 3, 5, 7, 9, 11, 13, 15, 0}));

  darray1.shrink(4);
  THES_REQUIRE(darray1.size() == 4);
  THES_CHECK(test::range_eq(darray1, std::array{1, 3, 5, 7}));

  darray1.clear();
  THES_CHECK(darray1.size() == 0);
  THES_CHECK(darray1.empty());
  THES_CHECK(darray1.allocation_size() == 16);
}

THES_TEST_CASE("DynamicArray: clear_memory resets state safely", "[containers][array][dynamic]") {
  thes::DynamicArray<int, thes::DefaultInit> darray(4);
  darray.clear_memory();
  THES_CHECK(darray.size() == 0);
  THES_CHECK(darray.allocation_size() == 0);
  // `darray` is safely destructible here now that `TypedChunk::deallocate` nulls its pointers.
}

THES_TEST_CASE("DynamicArray: default-init, resize, push_back, pop_back",
               "[containers][array][dynamic]") {
  thes::DynamicArray<int, thes::DefaultInit> darray2(3);
  darray2[2] = 3;
  THES_REQUIRE(darray2.size() == 3);
  THES_CHECK(darray2[2] == 3);
  THES_CHECK(darray2.front() == darray2[0]);
  THES_CHECK(darray2.back() == 3);

  darray2.resize(7);
  for (const auto i : thes::views::indices<std::size_t>(3, 7)) {
    darray2[i] = static_cast<int>(3 * i + 2);
  }
  THES_REQUIRE(darray2.size() == 7);
  THES_CHECK(darray2.allocation_size() == 8);
  THES_CHECK(test::range_eq(darray2, std::array{darray2[0], darray2[1], 3, 11, 14, 17, 20}));

  for (const int i : thes::views::indices<int>(4)) {
    darray2.push_back(i);
  }
  THES_REQUIRE(darray2.size() == 11);
  THES_CHECK(darray2.allocation_size() == 16);
  THES_CHECK(
    test::range_eq(darray2, std::array{darray2[0], darray2[1], 3, 11, 14, 17, 20, 0, 1, 2, 3}));

  for ([[maybe_unused]] const int i : thes::views::indices(4)) {
    darray2.pop_back();
  }
  THES_REQUIRE(darray2.size() == 7);
  THES_CHECK(darray2.allocation_size() == 16);
  THES_CHECK(test::range_eq(darray2, std::array{darray2[0], darray2[1], 3, 11, 14, 17, 20}));
}

THES_TEST_CASE("DynamicArray: initializer_list construction", "[containers][array][dynamic]") {
  thes::DynamicArray<int, thes::DefaultInit> darray{1, 2, 3, 4};
  THES_REQUIRE(darray.size() == 4);
  THES_CHECK(test::range_eq(darray, std::array{1, 2, 3, 4}));
}

THES_TEST_CASE("DynamicArray: equality compares contents, not capacity",
               "[containers][array][dynamic]") {
  thes::DynamicArray<int, thes::DefaultInit> a{1, 2, 3};
  thes::DynamicArray<int, thes::DefaultInit> b{1, 2, 3};
  b.reserve(64);
  thes::DynamicArray<int, thes::DefaultInit> c{1, 2, 4};

  THES_CHECK(a == b);
  THES_CHECK(a != c);
}

THES_TEST_CASE("DynamicArray: ADL swap exchanges contents", "[containers][array][dynamic]") {
  thes::DynamicArray<int, thes::DefaultInit> a{1, 2, 3};
  thes::DynamicArray<int, thes::DefaultInit> b{9, 8};

  using std::swap;
  swap(a, b);

  THES_CHECK(test::range_eq(a, std::array{9, 8}));
  THES_CHECK(test::range_eq(b, std::array{1, 2, 3}));
}

THES_TEST_CASE("DynamicArray: copy construction and copy assignment are independent",
               "[containers][array][dynamic]") {
  thes::DynamicArray<int, thes::DefaultInit> original{1, 2, 3};

  thes::DynamicArray<int, thes::DefaultInit> copy_ctor{original};
  THES_REQUIRE(copy_ctor.size() == original.size());
  THES_CHECK(copy_ctor == original);

  copy_ctor[0] = 99;
  THES_CHECK(original[0] == 1);
  THES_CHECK(copy_ctor[0] == 99);
  THES_CHECK(copy_ctor != original);

  thes::DynamicArray<int, thes::DefaultInit> copy_assign(1);
  copy_assign = original;
  THES_REQUIRE(copy_assign.size() == original.size());
  THES_CHECK(copy_assign == original);
  copy_assign[1] = 42;
  THES_CHECK(original[1] == 2);
}

THES_TEST_CASE("DynamicArray: move construction and move assignment leave source empty",
               "[containers][array][dynamic]") {
  thes::DynamicArray<int, thes::DefaultInit> src{1, 2, 3};

  thes::DynamicArray<int, thes::DefaultInit> moved{std::move(src)};
  THES_REQUIRE(moved.size() == 3);
  THES_CHECK(test::range_eq(moved, std::array{1, 2, 3}));

  thes::DynamicArray<int, thes::DefaultInit> src2{4, 5};
  thes::DynamicArray<int, thes::DefaultInit> move_assigned(1);
  move_assigned = std::move(src2);
  THES_REQUIRE(move_assigned.size() == 2);
  THES_CHECK(test::range_eq(move_assigned, std::array{4, 5}));
}

THES_TEST_CASE("DynamicArray: erase single element and range", "[containers][array][dynamic]") {
  thes::DynamicArray<int, thes::DefaultInit> darray{0, 1, 2, 3, 4, 5};

  darray.erase(darray.begin() + 2);
  THES_REQUIRE(darray.size() == 5);
  THES_CHECK(test::range_eq(darray, std::array{0, 1, 3, 4, 5}));

  darray.erase(darray.begin() + 1, darray.begin() + 3);
  THES_REQUIRE(darray.size() == 3);
  THES_CHECK(test::range_eq(darray, std::array{0, 4, 5}));
}

THES_TEST_CASE("DynamicArray: insert in the middle without reallocation",
               "[containers][array][dynamic]") {
  thes::DynamicArray<int, thes::DefaultInit> darray(2);
  darray.reserve(8);
  darray[0] = 1;
  darray[1] = 2;
  darray.resize(2);

  darray.insert(darray.begin() + 1, 99);
  THES_REQUIRE(darray.size() == 3);
  THES_CHECK(test::range_eq(darray, std::array{1, 99, 2}));
}

THES_TEST_CASE("DynamicArray: insert triggers reallocation", "[containers][array][dynamic]") {
  thes::DynamicArray<int, thes::DefaultInit> darray{1, 2, 3};
  THES_REQUIRE(darray.allocation_size() == 3);

  darray.insert(darray.begin() + 1, 42);
  THES_REQUIRE(darray.size() == 4);
  THES_CHECK(darray.allocation_size() >= 4);
  THES_CHECK(test::range_eq(darray, std::array{1, 42, 2, 3}));
}

THES_TEST_CASE("DynamicArray: value-init destroys class-typed elements",
               "[containers][array][dynamic]") {
  {
    thes::DynamicArray<S, thes::DefaultInit> darray3(3);
    THES_REQUIRE(darray3.size() == 3);
    THES_CHECK(darray3.allocation_size() == 3);
    THES_CHECK(test::range_eq(darray3, std::array{S{}, S{}, S{}}));
    S::counter() = 0;
  }
  THES_CHECK(S::counter() == 3);
}

THES_TEST_CASE("DynamicArray: no-init manual construction and emplace_back",
               "[containers][array][dynamic]") {
  thes::DynamicArray<S, thes::NoInit> darray4(3);
  THES_REQUIRE(darray4.size() == 3);
  THES_CHECK(darray4.allocation_size() == 3);

  for (const auto i : thes::views::indices<std::size_t>(3)) {
    darray4.initial_emplace(i);
  }
  THES_REQUIRE(darray4.size() == 3);
  THES_CHECK(darray4.allocation_size() == 3);
  THES_CHECK(test::range_eq(darray4, std::array{S{}, S{}, S{}}));

  S::counter() = 0;
  for (const int i : thes::views::indices<int>(6)) {
    darray4.emplace_back(i);
  }
  // 3 are destructed when adding “0” (at capacity 3)
  // 8 are destructed when adding “5” (at capacity 8)
  THES_CHECK(S::counter() == 11);
  THES_REQUIRE(darray4.size() == 9);
  THES_CHECK(darray4.allocation_size() == 16);
  THES_CHECK(
    test::range_eq(darray4, std::array{S{}, S{}, S{}, S{0}, S{1}, S{2}, S{3}, S{4}, S{5}}));

  // fill with S{0}
  std::ranges::fill(darray4, S{0});
  THES_CHECK(
    test::range_eq(darray4, std::array{S{0}, S{0}, S{0}, S{0}, S{0}, S{0}, S{0}, S{0}, S{0}}));

  S::counter() = 0;
}

THES_TEST_CASE("DynamicArray: insert_any grows and initializes gap and padding",
               "[containers][array][dynamic]") {
  thes::DynamicArray<S, thes::ValueInit> darray5(5);
  darray5.reserve(16);
  for (const auto i : thes::views::indices(5ZU)) {
    darray5[i] = S{int(i)};
  }
  fmt::print("{}\n", darray5);

  darray5.insert_any(darray5.begin() + 2, 4, 3);
  fmt::print("{}\n", darray5);

  THES_REQUIRE(darray5.size() == 12);
  THES_CHECK(darray5[0] == S{0});
  THES_CHECK(darray5[1] == S{1});
}

//==================================================================================================
// FixedArray
//==================================================================================================

THES_TEST_CASE("FixedArray: value-init construction and element access",
               "[containers][array][fixed]") {
  thes::FixedArray<int, thes::ValueInit> farray(4);
  THES_REQUIRE(farray.size() == 4);
  THES_CHECK(!farray.empty());
  THES_CHECK(test::range_eq(farray, std::array{0, 0, 0, 0}));

  farray[0] = 10;
  farray[3] = 40;
  THES_CHECK(farray.front() == 10);
  THES_CHECK(farray.back() == 40);
}

THES_TEST_CASE("FixedArray: initializer_list construction and span", "[containers][array][fixed]") {
  thes::FixedArray<int, thes::DefaultInit> farray{5, 6, 7};
  THES_REQUIRE(farray.size() == 3);
  THES_CHECK(test::range_eq(farray, std::array{5, 6, 7}));

  auto sp = farray.span();
  THES_REQUIRE(sp.size() == 3);
  THES_CHECK(sp[1] == 6);
}

THES_TEST_CASE("FixedArray: at() throws std::out_of_range for invalid index",
               "[containers][array][fixed]") {
  thes::FixedArray<int, thes::ValueInit> farray(3);
  THES_CHECK(farray.at(0) == 0);
  THES_CHECK_THROWS_AS(farray.at(3), std::out_of_range);
  THES_CHECK_THROWS_AS(farray.at(100), std::out_of_range);

  const auto& const_farray = farray;
  THES_CHECK_THROWS_AS(const_farray.at(3), std::out_of_range);
}

THES_TEST_CASE("FixedArray: copy is independent of original, equality compares contents",
               "[containers][array][fixed]") {
  thes::FixedArray<int, thes::DefaultInit> original{1, 2, 3};
  thes::FixedArray<int, thes::DefaultInit> copy{original};
  THES_REQUIRE(copy.size() == original.size());
  THES_CHECK(copy == original);

  copy[0] = 99;
  THES_CHECK(original[0] == 1);
  THES_CHECK(copy[0] == 99);
  THES_CHECK(copy != original);
}

THES_TEST_CASE("FixedArray: move construction transfers ownership", "[containers][array][fixed]") {
  thes::FixedArray<int, thes::DefaultInit> src{1, 2, 3};
  thes::FixedArray<int, thes::DefaultInit> moved{std::move(src)};
  THES_REQUIRE(moved.size() == 3);
  THES_CHECK(test::range_eq(moved, std::array{1, 2, 3}));
}

THES_TEST_CASE("FixedArray: ADL swap exchanges contents", "[containers][array][fixed]") {
  thes::FixedArray<int, thes::DefaultInit> a{1, 2, 3};
  thes::FixedArray<int, thes::DefaultInit> b{9, 8, 7};

  using std::swap;
  swap(a, b);

  THES_CHECK(test::range_eq(a, std::array{9, 8, 7}));
  THES_CHECK(test::range_eq(b, std::array{1, 2, 3}));
}

//==================================================================================================
// FixedAllocArray
//==================================================================================================

THES_TEST_CASE("FixedAllocArray: create_with_capacity, emplace_back, pop_back",
               "[containers][array][fixed-alloc]") {
  auto farray = thes::FixedAllocArray<int>::create_with_capacity(4);
  THES_CHECK(farray.empty());
  THES_CHECK(farray.allocation_size() == 4);

  farray.emplace_back(1);
  farray.emplace_back(2);
  farray.push_back(3);
  THES_REQUIRE(farray.size() == 3);
  THES_CHECK(!farray.empty());
  THES_CHECK(test::range_eq(farray, std::array{1, 2, 3}));
  THES_CHECK(farray.front() == 1);
  THES_CHECK(farray.back() == 3);

  farray.pop_back();
  THES_REQUIRE(farray.size() == 2);
  THES_CHECK(test::range_eq(farray, std::array{1, 2}));
}

THES_TEST_CASE("FixedAllocArray: clear destroys elements and resets size",
               "[containers][array][fixed-alloc]") {
  auto farray = thes::FixedAllocArray<S>::create_with_capacity(3);
  farray.emplace_back(1);
  farray.emplace_back(2);
  S::counter() = 0;
  farray.clear();
  THES_CHECK(S::counter() == 2);
  THES_CHECK(farray.size() == 0);
  THES_CHECK(farray.empty());
}

THES_TEST_CASE("FixedAllocArray: initializer_list construction and reverse iteration",
               "[containers][array][fixed-alloc]") {
  thes::FixedAllocArray<int> farray{1, 2, 3};
  THES_REQUIRE(farray.size() == 3);
  THES_CHECK(test::range_eq(farray, std::array{1, 2, 3}));

  // Now correctly `rbegin() == reverse_iterator{end()}` and `rend() == reverse_iterator{begin()}`.
  std::vector<int> reversed(farray.rbegin(), farray.rend());
  THES_CHECK(test::range_eq(reversed, std::array{3, 2, 1}));

  const auto& const_farray = farray;
  std::vector<int> const_reversed(const_farray.rbegin(), const_farray.rend());
  THES_CHECK(test::range_eq(const_reversed, std::array{3, 2, 1}));
}

THES_TEST_CASE("FixedAllocArray: equality and ADL swap", "[containers][array][fixed-alloc]") {
  thes::FixedAllocArray<int> a{1, 2, 3};
  thes::FixedAllocArray<int> b{1, 2, 3};
  thes::FixedAllocArray<int> c{4, 5};

  THES_CHECK(a == b);
  THES_CHECK(a != c);

  using std::swap;
  swap(a, c);
  THES_CHECK(test::range_eq(a, std::array{4, 5}));
  THES_CHECK(test::range_eq(c, std::array{1, 2, 3}));
}

//==================================================================================================
// LimitedArray
//==================================================================================================

THES_TEST_CASE("LimitedArray: default construction, size, capacity",
               "[containers][array][limited]") {
  thes::LimitedArray<int, 5> larray(3);
  THES_CHECK(larray.capacity == 5);
  THES_CHECK(larray.data() != nullptr);
}

THES_TEST_CASE("LimitedArray: variadic construction and iteration",
               "[containers][array][limited]") {
  thes::LimitedArray<int, 4> larray(1, 2, 3);
  THES_CHECK(test::range_eq(larray, std::array{1, 2, 3}));
}

THES_TEST_CASE("LimitedArray: iterator-pair construction", "[containers][array][limited]") {
  std::array<int, 3> src{7, 8, 9};
  thes::LimitedArray<int, 5> larray(src.begin(), src.end());
  THES_CHECK(test::range_eq(larray, src));
}

THES_TEST_CASE("LimitedArray: mutable iteration modifies elements in place",
               "[containers][array][limited]") {
  thes::LimitedArray<int, 4> larray(1, 2, 3);
  for (auto& x : larray) {
    x *= 10;
  }
  THES_CHECK(test::range_eq(larray, std::array{10, 20, 30}));

  std::ranges::sort(larray, std::greater{});
  THES_CHECK(test::range_eq(larray, std::array{30, 20, 10}));
}

THES_TEST_CASE("LimitedArray: push_back, pop_back, clear", "[containers][array][limited]") {
  thes::LimitedArray<int, 4> larray;
  THES_CHECK(larray.empty());

  larray.push_back(1);
  larray.push_back(2);
  THES_REQUIRE(larray.size() == 2);
  THES_CHECK(test::range_eq(larray, std::array{1, 2}));

  larray.pop_back();
  THES_REQUIRE(larray.size() == 1);
  THES_CHECK(larray.front() == 1);

  larray.clear();
  THES_CHECK(larray.empty());
}

THES_TEST_CASE("LimitedArray: equality compares only the logical size",
               "[containers][array][limited]") {
  thes::LimitedArray<int, 4> a(1, 2, 3);
  thes::LimitedArray<int, 4> b(1, 2, 3);
  thes::LimitedArray<int, 4> c(1, 2);
  THES_CHECK(a == b);
  THES_CHECK(a != c);
}

THES_TEST_CASE("LimitedArray: ADL swap exchanges contents", "[containers][array][limited]") {
  thes::LimitedArray<int, 4> a(1, 2, 3);
  thes::LimitedArray<int, 4> b(9, 8);

  using std::swap;
  swap(a, b);

  THES_CHECK(test::range_eq(a, std::array{9, 8}));
  THES_CHECK(test::range_eq(b, std::array{1, 2, 3}));
}
} // namespace

THES_TEST_MAIN()
