#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_GENERATORS_COMBINED_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_GENERATORS_COMBINED_HPP

#include <utility>

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"

namespace thes::star {
template<typename TRangeGen1, typename TRangeGen2>
requires(IsRangeGenerator<std::decay_t<TRangeGen1>> && IsRangeGenerator<std::decay_t<TRangeGen2>>)
struct CombinedGenerator {
  TRangeGen1 gen1;
  TRangeGen2 gen2;

  template<typename... TRanges>
  constexpr auto operator()(TRanges&&... ranges) const {
    return gen2(gen1(std::forward<TRanges>(ranges)...));
  }
};

template<typename TRangeGen1, typename TRangeGen2>
struct RangeGeneratorTrait<CombinedGenerator<TRangeGen1, TRangeGen2>> : public std::true_type {};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_GENERATORS_COMBINED_HPP
