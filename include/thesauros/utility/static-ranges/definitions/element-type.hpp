#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_ELEMENT_TYPE_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_ELEMENT_TYPE_HPP

#include <cstddef>

#include "thesauros/utility/static-ranges/definitions/get-at.hpp"
#include "thesauros/utility/type-sequence.hpp"
#include "thesauros/utility/value-sequence.hpp"

#include "size.hpp"

namespace thes::star {
template<typename TRange>
struct ElementTypesTrait {
  using Types = decltype([]<std::size_t... tIdxs>(ValueSequence<std::size_t, tIdxs...> /*idxs*/) {
    return TypeSeq<std::decay_t<decltype(get_at<tIdxs>(std::declval<const TRange&>()))>...>{};
  }(MakeIntegerSequence<std::size_t, 0, size<TRange>>{}));
};

template<typename TRange>
concept HasSingleElementType = ElementTypesTrait<TRange>::Types::is_unique;

template<typename TRange>
requires HasSingleElementType<TRange>
struct ElementTypeTrait {
  using Value = typename ElementTypesTrait<TRange>::Types::Unique;
};

template<typename TRange>
using ElementType = typename ElementTypeTrait<TRange>::Value;
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_ELEMENT_TYPE_HPP
