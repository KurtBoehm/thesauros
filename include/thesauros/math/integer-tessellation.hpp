#ifndef INCLUDE_THESAUROS_MATH_INTEGER_TESSELLATION_HPP
#define INCLUDE_THESAUROS_MATH_INTEGER_TESSELLATION_HPP

#include <array>
#include <concepts>
#include <cstddef>
#include <limits>

#include "thesauros/static-ranges.hpp"
#include "thesauros/utility/value-tag.hpp"

namespace thes {
template<std::size_t tDimNum, std::unsigned_integral TInt>
inline std::array<TInt, tDimNum> box_tesselate(TInt tile_num, std::array<TInt, tDimNum> box_dims) {
  using Cost = double;
  struct Sol {
    std::array<TInt, tDimNum> sol{};
    Cost cost = std::numeric_limits<Cost>::infinity();
  };

  if constexpr (tDimNum == 1) {
    return {tile_num};
  }

  Sol best{};
  auto op = [&](auto op, Sol& sol, TInt remaining, thes::AnyIndexTag auto dim) {
    if constexpr (dim + 1 == tDimNum) {
      std::get<dim>(sol.sol) = remaining;
      const auto [min, max] =
        thes::star::transform([](TInt box_dim, TInt cnum) { return Cost(box_dim) / Cost(cnum); },
                              box_dims, sol.sol) |
        thes::star::minmax;
      sol.cost = max / min;
      if (sol.cost < best.cost) {
        best = sol;
      }
    } else {
      for (TInt dcnum = remaining; dcnum > 0; --dcnum) {
        if (remaining % dcnum == 0) {
          std::get<dim>(sol.sol) = dcnum;
          op(op, sol, remaining / dcnum, thes::index_tag<dim + 1>);
        }
      }
    }
  };
  Sol out{};
  op(op, out, tile_num, thes::index_tag<0>);
  return best.sol;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_MATH_INTEGER_TESSELLATION_HPP
