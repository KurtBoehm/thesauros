#include <compare>
#include <tuple>

#include "thesauros/utility/tuple.hpp"

int main() {
  constexpr std::tuple<int, float> ta1{1, 2};
  constexpr std::tuple<int, float> tb1{3, 1};
  constexpr auto cmp1 = ta1 <=> tb1;
  static_assert(ta1 != tb1);
  static_assert(ta1 < tb1);
  static_assert(cmp1 == std::partial_ordering::less);

  constexpr thes::Tuple<int, float> ta2{1, 2.F};
  constexpr thes::Tuple<int, float> tb2{3, 1.F};
  constexpr auto cmp2 = ta2 <=> tb2;
  static_assert(ta2 != tb2);
  static_assert(ta2 < tb2);
  static_assert(cmp2 == std::partial_ordering::less);
}
