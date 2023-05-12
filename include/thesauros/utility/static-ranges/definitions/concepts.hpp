#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_CONCEPTS_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_CONCEPTS_HPP

#include <cstddef>
#include <utility>

#include "get-at.hpp"
#include "size.hpp"

#include "thesauros/utility/value-sequence.hpp"

namespace thes::star {
template<typename TRange, std::size_t tIdx>
concept SupportsGetAt = requires(const TRange& range) { get_at<tIdx>(range); };

template<typename TRange>
concept IsStaticRange =
  requires { size<TRange>; } && []<std::size_t... tIdxs>(std::index_sequence<tIdxs...>) {
    return (... && SupportsGetAt<TRange, tIdxs>);
  }(std::make_index_sequence<size<TRange>>{});

template<typename TGen>
struct RangeGeneratorTrait : std::false_type {};
template<typename TGen>
struct ConsumerGeneratorTrait : std::false_type {};

template<typename TGen>
concept IsRangeGenerator = RangeGeneratorTrait<std::decay_t<TGen>>::value;
template<typename TGen>
concept IsConsumerGenerator = ConsumerGeneratorTrait<std::decay_t<TGen>>::value;
template<typename TGen>
concept IsPipeSink = IsRangeGenerator<TGen> || IsConsumerGenerator<TGen>;
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_CONCEPTS_HPP
