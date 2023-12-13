#include <array>
#include <cstddef>
#include <functional>
#include <iostream>
#include <numeric>

#include "thesauros/ranges.hpp"
#include "thesauros/test.hpp"
#include "thesauros/utility.hpp"

int main() {
  {
    static constexpr auto r = thes::range(10);
    static_assert(r.contains(9));

    for (auto it = r.begin(); it != r.end(); ++it) {
      std::cout << *it << '\n';
    }

    static_assert([] {
      auto it = r.begin();
      return *(++it);
    }() == 1);
    static_assert([] {
      auto it = r.begin();
      return *(it++);
    }() == 0);

    static_assert(r.begin()[4] == 4);
    static_assert(*(r.begin() + 4) == 4);
    static_assert(std::reduce(r.begin(), r.end()) == 45);
  }

  {
    static constexpr auto r = thes::reversed(thes::range(10));

    for (auto it = r.begin(); it != r.end(); ++it) {
      std::cout << *it << '\n';
    }

    static_assert([] {
      auto it = r.begin();
      return *(++it);
    }() == 8);
    static_assert([] {
      auto it = r.begin();
      return *(it++);
    }() == 9);

    static_assert(r.begin()[4] == 5);
    static_assert(*(r.begin() + 4) == 5);
    static_assert(std::reduce(r.begin(), r.end()) == 45);
  }

  {
    static constexpr auto r_base = thes::range(10);
    static constexpr auto r = thes::transform_range([](auto v) { return 2 * v; }, r_base);
    static_assert(thes::test::range_eq(r, std::array{0, 2, 4, 6, 8, 10, 12, 14, 16, 18}));
    static_assert(r.begin()[1] == 2);
  }

  {
    static constexpr auto r_base = thes::range(10);
    static constexpr auto r_trans = thes::transform_range([](auto v) { return 2 * v; }, r_base);
    static constexpr auto r = thes::value_range(r_trans.begin(), r_trans.end());
    static_assert(thes::test::range_eq(r, std::array{0, 2, 4, 6, 8, 10, 12, 14, 16, 18}));
    static_assert(r.begin()[1] == 2);
  }

  {
    static constexpr std::array base{8, 4, 6, 2};
    static constexpr auto enu = thes::enumerate<std::size_t>(base);
    using Value = decltype(enu)::Value;
    static_assert(thes::test::range_eq(
      enu, std::array{Value{0, base[0]}, Value{1, base[1]}, Value{2, base[2]}, Value{3, base[3]}}));
  }

  {
    static constexpr std::array v1{8, 4};
    static constexpr std::array v2{6, 2};
    static constexpr auto zip = thes::zip(v1, v2);
    static_assert(zip.size() == 2);
    static_assert(
      thes::test::range_eq(zip, std::array{thes::Tuple{v1[0], v2[0]}, thes::Tuple{v1[1], v2[1]}}));
  }

  {
    static constexpr auto r_base = thes::range(10);
    static constexpr auto r = thes::transform_range([](auto v) { return 2 * v; }, r_base);
    static_assert(thes::reduce(r, 0, std::plus{}) == 90);
  }
}
