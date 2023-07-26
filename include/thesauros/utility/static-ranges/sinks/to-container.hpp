#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_TO_CONTAINER_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_TO_CONTAINER_HPP

#include <cstddef>
#include <utility>

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"

namespace thes::star {
template<typename TContainer>
struct ToContainerGenerator : public ConsumerGeneratorBase {
  template<typename TRange>
  constexpr auto operator()(TRange&& range) const {
    return [&]<std::size_t... tIdxs>(std::index_sequence<tIdxs...> /*idxd*/) {
      return TContainer{get_at<tIdxs>(range)...};
    }(std::make_index_sequence<size<TRange>>{});
  }
};

template<typename TContainer>
inline constexpr ToContainerGenerator<TContainer> to_container{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_TO_CONTAINER_HPP
