#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_APPLY_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_APPLY_HPP

#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"

namespace thes::star {
template<typename TOp>
struct ApplyGenerator {
  TOp op;

  template<typename TRange>
  constexpr decltype(auto) operator()(TRange&& range) const {
    constexpr std::size_t size = thes::star::size<TRange>;
    return [this, &range]<std::size_t... tIdxs>(
             std::index_sequence<tIdxs...> /*idxs*/) -> decltype(auto) {
      return op(get_at<tIdxs>(range)...);
    }(std::make_index_sequence<size>{});
  }
};
template<typename TOp>
struct ConsumerGeneratorTrait<ApplyGenerator<TOp>> : public std::true_type {};

template<typename TOp>
inline constexpr ApplyGenerator<TOp> apply(TOp&& op) {
  return {std::forward<TOp>(op)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_APPLY_HPP
