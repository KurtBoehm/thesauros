#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_TO_CONTAINER_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_TO_CONTAINER_HPP

#include <cstddef>

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/value-sequence.hpp"

namespace thes::star {
template<typename TContainer>
struct ToContainerGenerator {
  template<typename TRange>
  constexpr auto operator()(TRange&& range) const {
    return [&range]<std::size_t... tIndices>(ValueSequence<std::size_t, tIndices...> /*idxd*/) {
      return TContainer{get_at<tIndices>(range)...};
    }
    (MakeIntegerSequence<std::size_t, 0, size<TRange>>{});
  }
};
template<typename TContainer>
struct ConsumerGeneratorTrait<ToContainerGenerator<TContainer>> : public std::true_type {};

template<typename TContainer>
inline constexpr ToContainerGenerator<TContainer> to_container{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_TO_CONTAINER_HPP
