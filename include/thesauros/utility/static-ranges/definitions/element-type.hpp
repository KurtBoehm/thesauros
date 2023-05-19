#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_ELEMENT_TYPE_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_ELEMENT_TYPE_HPP

#include <cstddef>
#include <utility>

#include "thesauros/utility/static-ranges/definitions/get-at.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/type-sequence.hpp"
#include "thesauros/utility/type-tag.hpp"

namespace thes::star {
template<typename TRange>
struct ElementTypeSeqTrait {
  template<typename TIdxRange>
  struct Impl;

  template<std::size_t... tIdxs>
  struct Impl<std::index_sequence<tIdxs...>> {
    using Type = TypeSeq<std::decay_t<decltype(get_at<tIdxs>(std::declval<const TRange&>()))>...>;
  };

  using Type = typename Impl<std::make_index_sequence<size<TRange>>>::Type;
};

template<typename TRange>
using ElementTypeSeq = typename ElementTypeSeqTrait<TRange>::Type;

namespace detail {
template<typename TRange>
concept HasValue = requires { typename TRange::Value; };
template<typename TRange>
concept HasValueType = requires { typename TRange::value_type; };
template<typename TRange>
concept HasElemType = ElementTypeSeq<TRange>::is_unique;
} // namespace detail

template<typename TRange>
struct ElementTypeTrait;

template<typename TRange>
requires detail::HasValue<TRange>
struct ElementTypeTrait<TRange> {
  using Type = typename TRange::Value;
};
template<typename TRange>
requires(!detail::HasValue<TRange> && detail::HasValueType<TRange>)
struct ElementTypeTrait<TRange> {
  using Type = typename TRange::value_type;
};
template<typename TRange>
requires(!detail::HasValue<TRange> && !detail::HasValueType<TRange> && detail::HasElemType<TRange>)
struct ElementTypeTrait<TRange> {
  using Type = typename ElementTypeSeq<TRange>::Unique;
};

template<typename TRange>
using ElementType = typename ElementTypeTrait<TRange>::Type;

template<typename TRange>
concept HasSingleElementType = requires { typename ElementType<TRange>; };
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_ELEMENT_TYPE_HPP
