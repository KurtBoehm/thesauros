#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_UNIQUE_VALUE_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_UNIQUE_VALUE_HPP

#include <functional>
#include <optional>
#include <utility>

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/element-type.hpp"
#include "thesauros/utility/static-ranges/definitions/get-at.hpp"

namespace thes::star {
struct HasUniqueValueGenerator {
  template<typename TRange>
  constexpr bool operator()(TRange&& range) const {
    constexpr std::size_t size = star::size<TRange>;
    if constexpr (size == 0 || !HasSingleElementType<TRange>) {
      return false;
    } else {
      return [&]<std::size_t... tIdxs>(std::index_sequence<tIdxs...>) {
        return (... && (get_at<0>(range) == get_at<tIdxs + 1>(range)));
      }(std::make_index_sequence<size - 1>{});
    }
  }
};
template<>
struct ConsumerGeneratorTrait<HasUniqueValueGenerator> : public std::true_type {};

inline constexpr HasUniqueValueGenerator has_unique_value{};

struct UniqueValueGenerator {
  template<typename TRange>
  requires HasSingleElementType<TRange>
  constexpr std::optional<ElementType<TRange>> operator()(TRange&& range) const {
    if (has_unique_value(range)) {
      return get_at<0>(range);
    }
    return std::nullopt;
  }
};
template<>
struct ConsumerGeneratorTrait<UniqueValueGenerator> : public std::true_type {};

inline constexpr UniqueValueGenerator unique_value{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_UNIQUE_VALUE_HPP
