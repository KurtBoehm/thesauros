#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_TO_STATIC_BITSET_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_TO_STATIC_BITSET_HPP

#include <concepts>
#include <cstddef>
#include <type_traits>

#include "thesauros/containers/static-bitset.hpp"
#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/static-ranges/definitions/type-traits.hpp"
#include "thesauros/utility/static-ranges/sinks/to-container.hpp"

namespace thes::star {
struct ToStaticBitsetGenerator : public ConsumerGeneratorBase {
  template<typename TRange>
  constexpr auto operator()(TRange&& range) const {
    using Range = std::decay_t<TRange>;
    static_assert(std::same_as<Value<TRange>, bool>);
    constexpr std::size_t size = thes::star::size<Range>;

    return to_container<StaticBitset<size>>(std::forward<TRange>(range));
  }
};

inline constexpr ToStaticBitsetGenerator to_static_bitset{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_TO_STATIC_BITSET_HPP
