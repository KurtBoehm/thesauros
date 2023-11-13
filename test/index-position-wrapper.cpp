#include <array>
#include <cstddef>

#include "thesauros/utility.hpp"

using Size = thes::u64;
static constexpr std::size_t dimension_num = 3;
using Size3 = std::array<Size, dimension_num>;
using IdPo = thes::IndexPositionWrapper<Size, dimension_num>;

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
    static_assert(incr3.index() == 6);
    static_assert(incr3.position() == Size3{0, 2, 0});
    static_assert(orig < incr3);

    static constexpr IdPo decr3 = -- -- --IdPo{incr3};
    static_assert(orig == decr3);
  }
  {
    static constexpr IdPo orig{23, {3, 3, 3}};
    static_assert(orig.index() == 23);
    static_assert(orig.position() == Size3{2, 1, 2});

    static constexpr IdPo incr3 = ++ ++ ++IdPo{orig};
    static_assert(incr3.index() == 26);
    static_assert(incr3.position() == Size3{2, 2, 2});

    static constexpr IdPo decr3 = -- -- --IdPo{incr3};
    static_assert(orig == decr3);
  }
}
