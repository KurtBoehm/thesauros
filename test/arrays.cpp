#include <array>
#include <cstddef>
#include <iostream>

#include "thesauros/containers.hpp"
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

int main() {
  thes::DynamicArrayValue<int> darray1(3);
  THES_ASSERT(darray1.size() == 3 && test::range_eq(darray1, std::array{0, 0, 0}));

  thes::DynamicArrayDefault<int> darray2(3);
  darray2[2] = 3;
  THES_ASSERT(darray2.size() == 3 && darray2[2] == 3);

  darray1.resize(9);
  for (const auto i : thes::range<std::size_t>(8)) {
    darray1[i] = static_cast<int>(2 * i + 1);
  }
  THES_ASSERT(darray1.size() == 9 && darray1.allocation_size() == 16);
  THES_ASSERT(test::range_eq(darray1, std::array{1, 3, 5, 7, 9, 11, 13, 15, 0}));

  darray2.resize(7);
  for (const auto i : thes::range<std::size_t>(3, 7)) {
    darray2[i] = static_cast<int>(3 * i + 2);
  }
  THES_ASSERT(darray2.size() == 7 && darray2.allocation_size() == 8);
  THES_ASSERT(test::range_eq(darray2, std::array{darray2[0], darray2[1], 3, 11, 14, 17, 20}));

  for (const int i : thes::range<int>(4)) {
    darray2.push_back(i);
  }
  THES_ASSERT(darray2.size() == 11 && darray2.allocation_size() == 16);
  THES_ASSERT(
    test::range_eq(darray2, std::array{darray2[0], darray2[1], 3, 11, 14, 17, 20, 0, 1, 2, 3}));

  for ([[maybe_unused]] const int i : thes::range(4)) {
    darray2.pop_back();
  }
  THES_ASSERT(darray2.size() == 7 && darray2.allocation_size() == 16);
  THES_ASSERT(test::range_eq(darray2, std::array{darray2[0], darray2[1], 3, 11, 14, 17, 20}));

  {
    thes::DynamicArrayDefault<S> darray3(3);
    THES_ASSERT(darray3.size() == 3 && darray3.allocation_size() == 3);
    THES_ASSERT(test::range_eq(darray3, std::array{S{}, S{}, S{}}));
    S::counter() = 0;
  }
  THES_ASSERT(S::counter() == 3);

  {
    thes::DynamicArrayUninit<S> darray4(3);
    THES_ASSERT(darray4.size() == 3 && darray4.allocation_size() == 3);

    for (const auto i : thes::range<std::size_t>(3)) {
      darray4.initial_emplace(i);
    }
    THES_ASSERT(darray4.size() == 3 && darray4.allocation_size() == 3);
    THES_ASSERT(test::range_eq(darray4, std::array{S{}, S{}, S{}}));

    S::counter() = 0;
    for (const int i : thes::range<int>(6)) {
      darray4.emplace_back(i);
    }
    // 3 are destructed when adding “0” (at capacity 3)
    // 8 are destructed when adding “5” (at capacity 8)
    THES_ASSERT(S::counter() == 11);
    THES_ASSERT(darray4.size() == 9 && darray4.allocation_size() == 16);
    THES_ASSERT(
      test::range_eq(darray4, std::array{S{}, S{}, S{}, S{0}, S{1}, S{2}, S{3}, S{4}, S{5}}));

    S::counter() = 0;
  }
  THES_ASSERT(S::counter() == 9);
}
