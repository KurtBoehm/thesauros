#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_FOR_EACH_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_FOR_EACH_HPP

#include <cstddef>
#include <utility>

#include "thesauros/optimization.hpp"
#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"

namespace thes::star {
template<typename TOp>
struct ForEachGenerator {
  TOp op;

  template<typename TRange>
  constexpr auto operator()(TRange&& range) const {
    constexpr std::size_t size = thes::star::size<TRange>;
    return [this, &range]<std::size_t... tIdxs>(std::index_sequence<tIdxs...> /*idxs*/) {
      return (op(get_at<tIdxs>(range)), ...);
    }(std::make_index_sequence<size>{});
  }
};
template<typename TOp>
struct ConsumerGeneratorTrait<ForEachGenerator<TOp>> : public std::true_type {};

template<typename TOp>
inline constexpr ForEachGenerator<TOp> for_each(TOp&& op) {
  return {std::forward<TOp>(op)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_FOR_EACH_HPP
