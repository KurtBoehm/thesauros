// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <array>
#include <cstddef>

#include "thesauros/types.hpp"
#include "thesauros/utility.hpp"

using Size = thes::u64;
static constexpr std::size_t dimension_num = 3;
using Size3 = std::array<Size, dimension_num>;
using IdPo = thes::PosIndexWrapper<Size, dimension_num>;
using DuIdPo = thes::DualPosIndexWrapper<Size, Size, dimension_num>;

int main() {
  {
    static constexpr IdPo orig{{0, 1, 0}, {3, 3, 3}};
    static_assert(orig.index() == 3);

    static constexpr IdPo incr = ++IdPo{orig};
    static_assert(incr.index() == 4);
    static_assert(incr.position() == Size3{0, 1, 1});

    static constexpr IdPo incr3 = ++(++IdPo{incr});
    static_assert(incr3 == orig + 3);
    static_assert(incr3 - 3 == orig);
    static_assert(incr3 - orig == 3);
    static_assert(incr3.index() == 6);
    static_assert(incr3.position() == Size3{0, 2, 0});
    static_assert(orig < incr3);

    static constexpr IdPo decr3 = -- -- --IdPo{incr3};
    static_assert(orig == decr3);

    static constexpr IdPo next_row = orig + 9;
    static_assert(next_row.index() == 12);
    static_assert(next_row.position() == Size3{1, 1, 0});
  }
  {
    static constexpr IdPo orig{23, {3, 3, 3}};
    static_assert(orig == 23);
    static_assert(orig.index() == 23);
    static_assert(orig.position() == Size3{2, 1, 2});

    static constexpr IdPo incr3 = ++ ++ ++IdPo{orig};
    static_assert(incr3.index() == 26);
    static_assert(incr3.position() == Size3{2, 2, 2});

    static constexpr IdPo decr3 = -- -- --IdPo{incr3};
    static_assert(orig == decr3);
    static_assert(decr3.position() == Size3{2, 1, 2});

    static constexpr IdPo after = ++IdPo{incr3};
    static_assert(after.index() == 27);
    static_assert(after.position() == Size3{3, 0, 0});
  }
  // Dual
  {
    static constexpr DuIdPo orig{0, {0, 1, 0}, {3, 3, 3}};
    static_assert(orig.index() == 0);

    static constexpr DuIdPo incr = ++DuIdPo{orig};
    static_assert(incr.index() == 1);
    static_assert(incr.position() == Size3{0, 1, 1});

    static constexpr DuIdPo incr3 = ++(++DuIdPo{incr});
    static_assert(incr3 == orig + 3);
    static_assert(incr3 - 3 == orig);
    static_assert(incr3 - orig == 3);
    static_assert(incr3.index() == 3);
    static_assert(incr3.position() == Size3{0, 2, 0});
    static_assert(orig < incr3);

    static constexpr DuIdPo decr3 = -- -- --DuIdPo{incr3};
    static_assert(orig == decr3);
  }
  {
    static constexpr DuIdPo orig{2, 23, {3, 3, 3}};
    static_assert(orig == 2);
    static_assert(orig.index() == 2);
    static_assert(orig.position() == Size3{2, 1, 2});

    static constexpr DuIdPo incr3 = ++ ++ ++DuIdPo{orig};
    static_assert(incr3.index() == 5);
    static_assert(incr3.position() == Size3{2, 2, 2});

    static constexpr DuIdPo decr3 = -- -- --DuIdPo{incr3};
    static_assert(orig == decr3);

    static constexpr DuIdPo after = ++DuIdPo{incr3};
    static_assert(after.index() == 6);
  }
}
