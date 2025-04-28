#ifndef INCLUDE_THESAUROS_STATIC_RANGES_PIPING_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_PIPING_HPP

#include <utility>

#include "thesauros/utility/inlining.hpp"
#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/generators/combined.hpp"

namespace thes::star {
template<AnyStaticRange TRange, IsPipeSink TRangeGen>
THES_ALWAYS_INLINE inline constexpr decltype(auto) operator|(TRange&& range, TRangeGen&& gen) {
  return std::forward<TRangeGen>(gen)(std::forward<TRange>(range));
}

template<IsRangeGenerator TRangeGen1, IsPipeSink TRangeGen2>
THES_ALWAYS_INLINE inline constexpr decltype(auto) operator|(TRangeGen1&& gen1, TRangeGen2&& gen2) {
  return CombinedGenerator<TRangeGen1, TRangeGen2>{std::forward<TRangeGen1>(gen1),
                                                   std::forward<TRangeGen2>(gen2)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_PIPING_HPP
