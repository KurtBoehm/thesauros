// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <compare>
#include <cstddef>
#include <ranges>
#include <vector>

#include "thesauros/iterator.hpp"
#include "thesauros/test.hpp"

namespace {
//====================================================================================================
// A random-access iterator with no external state, wrapping a raw pointer directly
//====================================================================================================

struct PointerIter : thes::IteratorFacade<thes::iter::DefaultTypes<int, std::ptrdiff_t>> {
  PointerIter() = default;
  explicit PointerIter(int* p) : ptr{p} {}

  [[nodiscard]] int& deref() const {
    return *ptr;
  }
  void incr() {
    ++ptr;
  }
  void decr() {
    --ptr;
  }
  [[nodiscard]] bool eq(const PointerIter& other) const {
    return ptr == other.ptr;
  }
  void iadd(std::ptrdiff_t n) {
    ptr += n;
  }
  [[nodiscard]] std::ptrdiff_t sub(const PointerIter& other) const {
    return ptr - other.ptr;
  }
  [[nodiscard]] std::strong_ordering three_way(const PointerIter& other) const {
    return ptr <=> other.ptr;
  }

  int* ptr = nullptr;
};

void test_pointer_like_random_access() {
  static_assert(std::random_access_iterator<PointerIter>);

  std::vector<int> data{1, 2, 3, 4, 5};
  PointerIter begin{data.data()};
  PointerIter end{data.data() + data.size()};

  THES_ALWAYS_ASSERT(*begin == 1);
  THES_ALWAYS_ASSERT(end - begin == 5);
  THES_ALWAYS_ASSERT(*(begin + 2) == 3);
  THES_ALWAYS_ASSERT(*(2 + begin) == 3);
  THES_ALWAYS_ASSERT(begin[3] == 4);

  PointerIter it = begin;
  ++it;
  THES_ALWAYS_ASSERT(*it == 2);
  it++;
  THES_ALWAYS_ASSERT(*it == 3);
  --it;
  THES_ALWAYS_ASSERT(*it == 2);
  it--;
  THES_ALWAYS_ASSERT(*it == 1);

  it += 4;
  THES_ALWAYS_ASSERT(*it == 5);
  it -= 4;
  THES_ALWAYS_ASSERT(*it == 1);

  THES_ALWAYS_ASSERT(begin < end);
  THES_ALWAYS_ASSERT(begin <= begin);
  THES_ALWAYS_ASSERT(end > begin);
  THES_ALWAYS_ASSERT(begin == PointerIter{data.data()});

  const auto sum = std::ranges::fold_left(begin, end, 0, std::plus{});
  THES_ALWAYS_ASSERT(sum == 15);
  THES_ALWAYS_ASSERT(std::ranges::equal(begin, end, data.begin(), data.end()));
}

//====================================================================================================
// A bidirectional-only iterator built on `StateFacade`, using a strided pointer as state
//====================================================================================================

struct StridedState {
  int* ptr = nullptr;
  std::ptrdiff_t stride = 1;

  constexpr StridedState& operator++() {
    ptr += stride;
    return *this;
  }
  constexpr StridedState& operator--() {
    ptr -= stride;
    return *this;
  }
  friend constexpr bool operator==(const StridedState& s1, const StridedState& s2) {
    return s1.ptr == s2.ptr;
  }
};

struct StridedIter : thes::StateIteratorFacade<thes::iter::DefaultTypes<int, std::ptrdiff_t>> {
  StridedIter() = default;
  StridedIter(int* ptr, std::ptrdiff_t stride) : state_{.ptr = ptr, .stride = stride} {}

  [[nodiscard]] int& value() const {
    return *state_.ptr;
  }
  [[nodiscard]] StridedState& state() {
    return state_;
  }
  [[nodiscard]] const StridedState& state() const {
    return state_;
  }

private:
  StridedState state_{};
};

void test_strided_bidirectional_with_state_facade() {
  static_assert(std::bidirectional_iterator<StridedIter>);
  static_assert(!std::random_access_iterator<StridedIter>);

  std::vector<int> data{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  StridedIter it{data.data(), 3};

  THES_ALWAYS_ASSERT(*it == 0);
  ++it;
  THES_ALWAYS_ASSERT(*it == 3);
  ++it;
  THES_ALWAYS_ASSERT(*it == 6);
  --it;
  THES_ALWAYS_ASSERT(*it == 3);

  StridedIter other{data.data() + 3, 3};
  THES_ALWAYS_ASSERT(it == other);
  ++other;
  THES_ALWAYS_ASSERT(it != other);
}

//====================================================================================================
// A value-returning random-access iterator with counting state and a custom `get_item`
//====================================================================================================

struct CountingState {
  int value = 0;
  int incr_calls = 0;
  int add_calls = 0;
  int decr_calls = 0;
  int sub_calls = 0;
  mutable int get_item_calls = 0;

  constexpr CountingState& operator++() {
    ++value;
    ++incr_calls;
    return *this;
  }
  constexpr CountingState& operator+=(std::ptrdiff_t n) {
    value += static_cast<int>(n);
    ++add_calls;
    return *this;
  }

  constexpr CountingState& operator--() {
    --value;
    ++decr_calls;
    return *this;
  }
  constexpr CountingState& operator-=(std::ptrdiff_t n) {
    value -= static_cast<int>(n);
    ++sub_calls;
    return *this;
  }

  friend constexpr std::ptrdiff_t operator-(const CountingState& s1, const CountingState& s2) {
    return s1.value - s2.value;
  }
  friend constexpr std::strong_ordering operator<=>(const CountingState& s1,
                                                    const CountingState& s2) {
    return s1.value <=> s2.value;
  }
};

struct CountingIter : thes::StateIteratorFacade<thes::iter::ValueTypes<int, std::ptrdiff_t>> {
  CountingIter() = default;
  explicit CountingIter(int value) : state_{.value = value} {}

  int value() const {
    return state_.value;
  }
  int get_item(std::ptrdiff_t n) const {
    ++state_.get_item_calls;
    return state_.value + static_cast<int>(n);
  }
  CountingState& state() {
    return state_;
  }
  const CountingState& state() const {
    return state_;
  }

private:
  CountingState state_{};
};

void test_counting_iterator_with_custom_get_item() {
  static_assert(std::random_access_iterator<CountingIter>);
  static_assert(std::same_as<std::iter_value_t<CountingIter>, int>);

  CountingIter it{10};
  THES_ALWAYS_ASSERT(*it == 10);
  THES_ALWAYS_ASSERT(it[5] == 15);
  THES_ALWAYS_ASSERT(it.state().get_item_calls == 1);

  it -= 3;
  THES_ALWAYS_ASSERT(*it == 7);
  THES_ALWAYS_ASSERT(it.state().sub_calls == 1);
  THES_ALWAYS_ASSERT(it.state().add_calls == 0);

  CountingIter other = it - 2;
  THES_ALWAYS_ASSERT(*other == 5);
  THES_ALWAYS_ASSERT(other.state().sub_calls == 2);

  THES_ALWAYS_ASSERT(it - other == 2);
  THES_ALWAYS_ASSERT(other < it);
  THES_ALWAYS_ASSERT(it > other);
}

//====================================================================================================
// A minimal forward-only iterator, verifying that no extra operations are required or exposed
//====================================================================================================

struct ForwardOnlyState {
  int value = 0;

  constexpr ForwardOnlyState& operator++() {
    ++value;
    return *this;
  }
  friend constexpr bool operator==(const ForwardOnlyState& s1, const ForwardOnlyState& s2) {
    return s1.value == s2.value;
  }
};

struct ForwardOnlyIter : thes::StateIteratorFacade<thes::iter::ValueTypes<int, std::ptrdiff_t>> {
  ForwardOnlyIter() = default;
  explicit ForwardOnlyIter(int value) : state_{value} {}

  [[nodiscard]] int value() const {
    return state_.value;
  }
  [[nodiscard]] ForwardOnlyState& state() {
    return state_;
  }
  [[nodiscard]] const ForwardOnlyState& state() const {
    return state_;
  }

private:
  ForwardOnlyState state_{};
};

void test_forward_only_iterator() {
  static_assert(std::forward_iterator<ForwardOnlyIter>);
  static_assert(!std::bidirectional_iterator<ForwardOnlyIter>);
  static_assert(!std::random_access_iterator<ForwardOnlyIter>);

  ForwardOnlyIter it{0};
  ForwardOnlyIter end{5};
  const auto sum = std::ranges::fold_left(it, end, 0, std::plus{});
  const auto expected = std::ranges::fold_left(std::views::iota(0, 5), 0, std::plus{});
  THES_ALWAYS_ASSERT(sum == expected);
}
} // namespace

int main() {
  test_pointer_like_random_access();
  test_strided_bidirectional_with_state_facade();
  test_counting_iterator_with_custom_get_item();
  test_forward_only_iterator();

  fmt::print("All tests passed.\n");
}
