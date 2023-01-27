#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_CONCEPTS_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_CONCEPTS_HPP

#include <cstddef>
#include <utility>

#include "get-at.hpp"
#include "size.hpp"

#include "thesauros/utility/value-sequence.hpp"

namespace thes::star {
template<typename TRange>
concept IsStaticRange =
  requires {
    size<TRange>;
    []<std::size_t... tIdxs>(const TRange& range, ValueSequence<std::size_t, tIdxs...>) {
      (..., get_at<tIdxs>(range));
    }
    (std::declval<const TRange&>(), MakeIntegerSequence<std::size_t, 0, size<TRange>>{});
  };

template<typename TRangeGenerator>
struct RangeGeneratorTrait : std::false_type {};
template<typename TRangeGenerator>
concept IsRangeGenerator = RangeGeneratorTrait<TRangeGenerator>::value;
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_CONCEPTS_HPP
