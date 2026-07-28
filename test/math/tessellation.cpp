// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <array>
#include <functional>

#include "thesauros/format.hpp"
#include "thesauros/math.hpp"
#include "thesauros/static-ranges.hpp"
#include "thesauros/test.hpp"

int main() {
  using Int = unsigned;
  for (Int i = 1; i < 256; ++i) {
    const auto tess = thes::box_tesselate(i, std::array<Int, 3>{1451, 1530, 1530});
    THES_ALWAYS_ASSERT((tess | thes::star::left_reduce(std::multiplies{})) == i);
    fmt::print("{}: {}\n", i, tess);
  }
}
