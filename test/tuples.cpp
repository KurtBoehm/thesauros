// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

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
