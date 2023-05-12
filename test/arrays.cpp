#include <iostream>

#include "thesauros/containers.hpp"
#include "thesauros/format.hpp"
#include "thesauros/ranges.hpp"

#include "tools.hpp"

// namespace fmt = thes::fmt;
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

  // friend std::ostream& operator<<(std::ostream& stream, const S& self) {
  //   return stream << "S(" << self.i << ")";
  // }

  bool operator==(const S&) const = default;
};

int main() {
  thes::DynamicArrayValue<int> darray1(3);
  // std::cout << fmt::set(fmt::fg_red, thes::range_print(darray1)) << std::endl;
  THES_ASSERT(darray1.size() == 3 && test::rangeq(darray1, std::array{0, 0, 0}));

  thes::DynamicArrayDefault<int> darray2(3);
  darray2[2] = 3;
  // std::cout << fmt::set(fmt::fg_green, thes::range_print(darray2)) << std::endl;
  THES_ASSERT(darray2.size() == 3 && darray2[2] == 3);

  darray1.resize(9);
  for (const auto i : thes::range<std::size_t>(8)) {
    darray1[i] = static_cast<int>(2 * i + 1);
  }
  // std::cout << darray1.allocation_size() << std::endl;
  THES_ASSERT(darray1.size() == 9 && darray1.allocation_size() == 16);
  // std::cout << fmt::set(fmt::fg_red, thes::range_print(darray1)) << std::endl;
  THES_ASSERT(test::rangeq(darray1, std::array{1, 3, 5, 7, 9, 11, 13, 15, 0}));

  darray2.resize(7);
  for (const auto i : thes::range<std::size_t>(3, 7)) {
    darray2[i] = static_cast<int>(3 * i + 2);
  }
  // std::cout << darray2.allocation_size() << std::endl;
  THES_ASSERT(darray2.size() == 7 && darray2.allocation_size() == 8);
  // std::cout << fmt::set(fmt::fg_green, thes::range_print(darray2)) << std::endl;
  THES_ASSERT(test::rangeq(darray2, std::array{darray2[0], darray2[1], 3, 11, 14, 17, 20}));

  for (const int i : thes::range<int>(4)) {
    darray2.push_back(i);
  }
  // std::cout << darray2.allocation_size() << std::endl;
  THES_ASSERT(darray2.size() == 11 && darray2.allocation_size() == 16);
  // std::cout << fmt::set(fmt::fg_green, thes::range_print(darray2)) << std::endl;
  THES_ASSERT(
    test::rangeq(darray2, std::array{darray2[0], darray2[1], 3, 11, 14, 17, 20, 0, 1, 2, 3}));

  for ([[maybe_unused]] const int i : thes::range(4)) {
    darray2.pop_back();
  }
  // std::cout << darray2.allocation_size() << std::endl;
  THES_ASSERT(darray2.size() == 7 && darray2.allocation_size() == 16);
  // std::cout << fmt::set(fmt::fg_green, thes::range_print(darray2)) << std::endl;
  THES_ASSERT(test::rangeq(darray2, std::array{darray2[0], darray2[1], 3, 11, 14, 17, 20}));

  {
    thes::DynamicArrayDefault<S> darray3(3);
    // std::cout << darray3.allocation_size() << std::endl;
    // std::cout << fmt::set(fmt::fg_blue, thes::range_print(darray3)) << std::endl;
    THES_ASSERT(darray3.size() == 3 && darray3.allocation_size() == 3);
    THES_ASSERT(test::rangeq(darray3, std::array{S{}, S{}, S{}}));
    S::counter() = 0;
  }
  THES_ASSERT(S::counter() == 3);

  {
    thes::DynamicArrayUninit<S> darray4(3);
    // std::cout << darray4.allocation_size() << std::endl;
    THES_ASSERT(darray4.size() == 3 && darray4.allocation_size() == 3);
    // std::cout << fmt::set(fmt::fg_cyan, thes::range_print(darray4)) << std::endl;

    for (const auto i : thes::range<std::size_t>(3)) {
      // std::cout << fmt::set(fmt::italic, i) << std::endl;
      darray4.initial_emplace(i);
    }
    // std::cout << darray4.allocation_size() << std::endl;
    // std::cout << fmt::set(fmt::fg_cyan, thes::range_print(darray4)) << std::endl;
    THES_ASSERT(darray4.size() == 3 && darray4.allocation_size() == 3);
    THES_ASSERT(test::rangeq(darray4, std::array{S{}, S{}, S{}}));

    S::counter() = 0;
    for (const int i : thes::range<int>(6)) {
      // std::cout << fmt::set(fmt::italic, i) << std::endl;
      darray4.emplace_back(i);
    }
    // std::cout << darray4.allocation_size() << std::endl;
    // std::cout << fmt::set(fmt::fg_cyan, thes::range_print(darray4)) << std::endl;
    // 3 are destructed when adding “0” (at capacity 3)
    // 8 are destructed when adding “5” (at capacity 8)
    THES_ASSERT(S::counter() == 11);
    THES_ASSERT(darray4.size() == 9 && darray4.allocation_size() == 16);
    THES_ASSERT(
      test::rangeq(darray4, std::array{S{}, S{}, S{}, S{0}, S{1}, S{2}, S{3}, S{4}, S{5}}));

    S::counter() = 0;
  }
  THES_ASSERT(S::counter() == 9);
}
