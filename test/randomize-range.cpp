// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <random>
#include <ranges>

#include "thesauros/thesauros.hpp"

using Size = unsigned int;

void run(const Size size) {
  fmt::print("test {}\n", size);
  auto arr = std::views::iota(Size{0}, size);
  thes::DynamicArray<Size> perm(size);
  thes::RangeRandomizer rand{size, std::mt19937_64{size}};
  std::ranges::transform(arr, perm.begin(), [&](Size i) { return rand.transform(i); });
  if (size <= 48) {
    fmt::print("{}\n", perm);
  }
  std::ranges::sort(perm);
  THES_ASSERT(std::ranges::equal(arr, perm));
}

int main() {
  for (Size size : thes::range<Size>(100)) {
    run(size);
  }
  run(1U << 10U);
  run((1U << 13U) + (1U << 3U));
  run(0x3a745cf);
}
