// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "thesauros/algorithms.hpp"
#include "thesauros/format.hpp"

int main() {
  int a = 5;
  int b = 6;
  fmt::print("{}; {}\n", a, b);

  thes::swap_or_equal<int>(a, b);
  fmt::print("{}; {}\n", a, b);

  thes::swap_or_equal<int&>(a, a);
  fmt::print("{}; {}\n", a, b);
}
