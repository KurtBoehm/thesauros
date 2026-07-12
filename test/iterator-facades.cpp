// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <compare>
#include <cstddef>
#include <ranges>
#include <vector>

#include "thesauros/iterator/facades.hpp"
#include "thesauros/test/test.hpp"

namespace {
//====================================================================================================
// A random-access iterator with no external state, wrapping a raw pointer directly
//====================================================================================================

struct PointerIter;

struct PointerIterProvider {
  using IterTypes = thes::iter_provider::DefaultTypes<int, std::ptrdiff_t>;

  static int& deref(const PointerIter& self);
  static void incr(PointerIter& self);
  static void decr(PointerIter& self);
  static bool eq(const PointerIter& d1, const PointerIter& d2);
  static void iadd(PointerIter& self, std::ptrdiff_t n);
  static std::ptrdiff_t sub(const PointerIter& d1, const PointerIter& d2);
  static std::strong_ordering three_way(const PointerIter& d1, const PointerIter& d2);
};

struct PointerIter : thes::IteratorFacade<PointerIter, PointerIterProvider> {
  PointerIter() = default;
  explicit PointerIter(int* p) : ptr{p} {}

  int* ptr = nullptr;
};

inline int& PointerIterProvider::deref(const PointerIter& self) {
  return *self.ptr;
}
inline void PointerIterProvider::incr(PointerIter& self) {
  ++self.ptr;
}
inline void PointerIterProvider::decr(PointerIter& self) {
  --self.ptr;
}
inline bool PointerIterProvider::eq(const PointerIter& d1, const PointerIter& d2) {
  return d1.ptr == d2.ptr;
}
inline void PointerIterProvider::iadd(PointerIter& self, std::ptrdiff_t n) {
  self.ptr += n;
}
inline std::ptrdiff_t PointerIterProvider::sub(const PointerIter& d1, const PointerIter& d2) {
  return d1.ptr - d2.ptr;
}
inline std::strong_ordering PointerIterProvider::three_way(const PointerIter& d1,
                                                           const PointerIter& d2) {
  return d1.ptr <=> d2.ptr;
}

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
// A bidirectional-only iterator using explicit facade state (a strided iterator)
//====================================================================================================

struct StridedState {
  int* ptr;
  std::ptrdiff_t stride;
};

struct StridedIterProvider {
  using IterTypes = thes::iter_provider::DefaultTypes<int, std::ptrdiff_t>;
  using FacadeState = StridedState;

  static int& deref(const StridedState& self) {
    return *self.ptr;
  }
  static void incr(StridedState& self) {
    self.ptr += self.stride;
  }
  static void decr(StridedState& self) {
    self.ptr -= self.stride;
  }
  static bool eq(const StridedState& d1, const StridedState& d2) {
    return d1.ptr == d2.ptr;
  }
};

struct StridedIter : thes::IteratorFacade<StridedIter, StridedIterProvider> {
  StridedIter() = default;
  StridedIter(int* ptr, std::ptrdiff_t stride) : state_{ptr, stride} {}

  StridedState& state() {
    return state_;
  }
  const StridedState& state() const {
    return state_;
  }

private:
  StridedState state_{};
};

void test_strided_bidirectional_with_facade_state() {
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
// A value-returning random-access iterator with custom `isub` and `get_item`
//====================================================================================================

struct CountingState {
  int value = 0;
  int add_calls = 0;
  int sub_calls = 0;
  mutable int get_item_calls = 0;
};

struct CountingIterProvider {
  using IterTypes = thes::iter_provider::ValueTypes<int, std::ptrdiff_t>;
  using FacadeState = CountingState;

  static int deref(const CountingState& self) {
    return self.value;
  }
  static void incr(CountingState& self) {
    self.value += 1;
  }
  static void decr(CountingState& self) {
    self.value -= 1;
  }
  static bool eq(const CountingState& d1, const CountingState& d2) {
    return d1.value == d2.value;
  }
  static void iadd(CountingState& self, std::ptrdiff_t n) {
    self.value += static_cast<int>(n);
    ++self.add_calls;
  }
  static void isub(CountingState& self, std::ptrdiff_t n) {
    self.value -= static_cast<int>(n);
    ++self.sub_calls;
  }
  static std::ptrdiff_t sub(const CountingState& d1, const CountingState& d2) {
    return d1.value - d2.value;
  }
  static std::strong_ordering three_way(const CountingState& d1, const CountingState& d2) {
    return d1.value <=> d2.value;
  }
  static int get_item(const CountingState& self, std::ptrdiff_t n) {
    ++self.get_item_calls;
    return self.value + static_cast<int>(n);
  }
};

struct CountingIter : thes::IteratorFacade<CountingIter, CountingIterProvider> {
  CountingIter() = default;
  explicit CountingIter(int value) : state_{.value = value} {}

  CountingState& state() {
    return state_;
  }
  const CountingState& state() const {
    return state_;
  }

private:
  CountingState state_{};
};

void test_value_iterator_with_custom_isub_and_get_item() {
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
};

struct ForwardOnlyProvider {
  using IterTypes = thes::iter_provider::ValueTypes<int, std::ptrdiff_t>;
  using FacadeState = ForwardOnlyState;

  static int deref(const ForwardOnlyState& self) {
    return self.value;
  }
  static void incr(ForwardOnlyState& self) {
    self.value += 1;
  }
  static bool eq(const ForwardOnlyState& d1, const ForwardOnlyState& d2) {
    return d1.value == d2.value;
  }
};

struct ForwardOnlyIter : thes::IteratorFacade<ForwardOnlyIter, ForwardOnlyProvider> {
  ForwardOnlyIter() = default;
  explicit ForwardOnlyIter(int value) : state_{value} {}

  ForwardOnlyState& state() {
    return state_;
  }
  const ForwardOnlyState& state() const {
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
  test_strided_bidirectional_with_facade_state();
  test_value_iterator_with_custom_isub_and_get_item();
  test_forward_only_iterator();

  fmt::print("All tests passed.\n");
}
