#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_TO_ARRAY_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_TO_ARRAY_HPP

#include <array>
#include <cstddef>
#include <type_traits>

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/static-ranges/definitions/type-traits.hpp"
#include "thesauros/utility/static-ranges/sinks/to-container.hpp"

namespace thes::star {
struct ToArrayGenerator : public ConsumerGeneratorBase {
  template<typename TRange>
  constexpr auto operator()(TRange&& range) const {
    using Range = std::decay_t<TRange>;
    using Value = star::Value<Range>;
    constexpr std::size_t size = thes::star::size<Range>;

    return to_container<std::array<Value, size>>(std::forward<TRange>(range));
  }
};

inline constexpr ToArrayGenerator to_array{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_TO_ARRAY_HPP
