#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_CONTAINS_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_CONTAINS_HPP

#include <concepts>
#include <type_traits>

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/sinks/apply.hpp"

namespace thes::star {
template<typename TValue>
struct ContainsGenerator : public ConsumerGeneratorBase {
  using Value = std::decay_t<TValue>;

  TValue value;

  explicit constexpr ContainsGenerator(TValue&& value) : value(std::forward<TValue>(value)) {}

  template<typename TRange>
  constexpr bool operator()(TRange&& range) const {
    return apply([&](const auto&... values) {
      return (... || [&]<typename TV>(const TV& v) {
        if constexpr (std::same_as<Value, std::decay_t<TV>>) {
          return value == v;
        } else {
          return false;
        }
      }(values));
    })(std::forward<TRange>(range));
  }
};

template<typename TValue>
inline constexpr auto contains(TValue&& value) {
  return ContainsGenerator<TValue>{std::forward<TValue>(value)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_CONTAINS_HPP
