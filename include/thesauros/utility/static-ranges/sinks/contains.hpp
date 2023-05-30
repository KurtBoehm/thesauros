#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_CONTAINS_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_CONTAINS_HPP

#include <concepts>
#include <type_traits>

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/sinks/apply.hpp"

namespace thes::star {
template<typename TValue>
struct ContainsGenerator {
  using Value = std::decay_t<TValue>;

  TValue value;

  template<typename TRange>
  constexpr bool operator()(TRange&& range) const {
    return apply(
      [&](const auto&... values) {
        return (... || [&]<typename TV>(const TV& v) {
          if constexpr (std::same_as<Value, std::decay_t<TV>>) {
            return value == v;
          } else {
            return false;
          }
        }(values));
      },
      range);
  }
};
template<typename TValue>
struct ConsumerGeneratorTrait<ContainsGenerator<TValue>> : public std::true_type {};

template<typename TValue>
inline constexpr ContainsGenerator<TValue> contains(TValue&& value) {
  return {std::forward<TValue>(value)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_CONTAINS_HPP
