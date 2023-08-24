#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_CONCEPTS_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_CONCEPTS_HPP

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "get-at.hpp"
#include "size.hpp"
#include "type-traits.hpp"

namespace thes::star {
template<typename TRange, std::size_t tIdx>
concept SupportsGetAt = requires(const TRange& range) { get_at<tIdx>(range); };

template<typename TRange>
concept AnyStaticRange =
  requires { size<TRange>; } && []<std::size_t... tIdxs>(std::index_sequence<tIdxs...>) {
    return (... && SupportsGetAt<TRange, tIdxs>);
  }(std::make_index_sequence<size<TRange>>{});

template<typename TRange>
concept AnyTypedStaticRange = AnyStaticRange<TRange> && HasValue<TRange>;
template<typename TRange, typename T>
concept TypedStaticRange = AnyTypedStaticRange<TRange> && std::same_as<Value<TRange>, T>;

struct RangeGeneratorBase {};
template<typename TGen>
struct RangeGeneratorTrait : std::is_base_of<RangeGeneratorBase, TGen> {};

struct ConsumerGeneratorBase {};
template<typename TGen>
struct ConsumerGeneratorTrait : std::is_base_of<ConsumerGeneratorBase, TGen> {};

template<typename TGen>
concept IsRangeGenerator = RangeGeneratorTrait<std::decay_t<TGen>>::value;
template<typename TGen>
concept IsConsumerGenerator = ConsumerGeneratorTrait<std::decay_t<TGen>>::value;
template<typename TGen>
concept IsPipeSink = IsRangeGenerator<TGen> || IsConsumerGenerator<TGen>;
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_DEFINITIONS_CONCEPTS_HPP
