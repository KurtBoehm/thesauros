#ifndef INCLUDE_THESAUROS_STATIC_RANGES_SINKS_UNIQUE_VALUE_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_SINKS_UNIQUE_VALUE_HPP

#include <cstddef>
#include <optional>
#include <utility>

#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/get-at.hpp"
#include "thesauros/static-ranges/definitions/type-traits.hpp"

namespace thes::star {
struct HasUniqueValueGenerator : public ConsumerGeneratorBase {
  template<typename TRange>
  constexpr bool operator()(TRange&& range) const {
    constexpr std::size_t size = star::size<TRange>;
    if constexpr (size == 0 || !HasValue<TRange>) {
      return false;
    } else {
      return [&]<std::size_t... tIdxs>(std::index_sequence<tIdxs...>) {
        return (... && (get_at<0>(range) == get_at<tIdxs + 1>(range)));
      }(std::make_index_sequence<size - 1>{});
    }
  }
};

inline constexpr HasUniqueValueGenerator has_unique_value{};

struct UniqueValueGenerator : public ConsumerGeneratorBase {
  template<HasValue TRange>
  constexpr std::optional<Value<TRange>> operator()(TRange&& range) const {
    if (has_unique_value(range)) {
      return get_at<0>(range);
    }
    return std::nullopt;
  }
};

inline constexpr UniqueValueGenerator unique_value{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_UNIQUE_VALUE_HPP
