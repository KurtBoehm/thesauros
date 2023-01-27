#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_PIPING_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_PIPING_HPP

#include <type_traits>

#include "thesauros/utility/static-ranges/generators/combined.hpp"

#include "definitions/concepts.hpp"

namespace thes::star {
template<typename TRange, typename TRangeGenerator>
requires(IsStaticRange<std::decay_t<TRange>> && IsRangeGenerator<std::decay_t<TRangeGenerator>>)
inline constexpr auto operator|(TRange&& range, TRangeGenerator&& gen) {
  return gen(std::forward<TRange>(range));
}

template<typename TRangeGen1, typename TRangeGen2>
requires(IsRangeGenerator<std::decay_t<TRangeGen1>> && IsRangeGenerator<std::decay_t<TRangeGen2>>)
inline constexpr auto operator|(TRangeGen1&& gen1, TRangeGen2&& gen2) {
  return CombinedGenerator<TRangeGen1, TRangeGen2>{std::forward<TRangeGen1>(gen1),
                                                   std::forward<TRangeGen2>(gen2)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_PIPING_HPP
