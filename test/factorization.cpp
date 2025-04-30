// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <ranges>

#include "thesauros/format.hpp"
#include "thesauros/math.hpp"
#include "thesauros/test.hpp"

int main() {
  using Int = unsigned;
  for (Int i = 1; i < 256; ++i) {
    const auto map = thes::factorize_map(i);
    const auto flat = thes::factorize_flat(i);
    auto flat_map = thes::factorize_map(i) | std::views::transform([](auto p) {
                      return std::views::iota(Int{0}, p.second) |
                             std::views::transform([&](auto) { return p.first; });
                    }) |
                    std::views::join;
    THES_ALWAYS_ASSERT(thes::test::range_eq(flat, flat_map));
    fmt::print("{}: {} / {}\n", i, map, flat);
  }
}
