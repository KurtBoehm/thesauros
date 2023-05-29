#include <array>
#include <iostream>
#include <numeric>

#include "thesauros/ranges.hpp"
#include "thesauros/test.hpp"

int main() {
  {
    constexpr auto r = thes::range(10);
    static_assert(r.contains(9));

    for (auto it = r.begin(); it != r.end(); ++it) {
      std::cout << *it << '\n';
    }

    static_assert([r] {
      auto it = r.begin();
      return *(++it);
    }() == 1);
    static_assert([r] {
      auto it = r.begin();
      return *(it++);
    }() == 0);

    static_assert(r.begin()[4] == 4);
    static_assert(*(r.begin() + 4) == 4);
    static_assert(std::reduce(r.begin(), r.end()) == 45);
  }

  {
    constexpr auto r = thes::reversed(thes::range(10));

    for (auto it = r.begin(); it != r.end(); ++it) {
      std::cout << *it << '\n';
    }

    static_assert([r] {
      auto it = r.begin();
      return *(++it);
    }() == 8);
    static_assert([r] {
      auto it = r.begin();
      return *(it++);
    }() == 9);

    static_assert(r.begin()[4] == 5);
    static_assert(*(r.begin() + 4) == 5);
    static_assert(std::reduce(r.begin(), r.end()) == 45);
  }

  {
    constexpr auto r_base = thes::range(10);
    constexpr auto r = thes::transform_range([](auto v) { return 2 * v; }, r_base);
    static_assert(thes::test::range_eq<false>(r, std::array{0, 2, 4, 6, 8, 10, 12, 14, 16, 18}));
    static_assert(r.begin()[1] == 2);
  }

  {
    constexpr auto r_base = thes::range(10);
    constexpr auto r_trans = thes::transform_range([](auto v) { return 2 * v; }, r_base);
    constexpr auto r = thes::value_range(r_trans.begin(), r_trans.end());
    static_assert(thes::test::range_eq<false>(r, std::array{0, 2, 4, 6, 8, 10, 12, 14, 16, 18}));
    static_assert(r.begin()[1] == 2);
  }
}
