#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_FOR_EACH_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_FOR_EACH_HPP

#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"

namespace thes::star {
template<typename TOp>
struct ForEachGenerator : public ConsumerGeneratorBase {
  TOp op;

  explicit constexpr ForEachGenerator(TOp&& op) : op(std::forward<TOp>(op)) {}

  template<typename TRange>
  constexpr auto operator()(TRange&& range) const {
    constexpr std::size_t size = thes::star::size<TRange>;
    return [this, &range]<std::size_t... tIdxs>(std::index_sequence<tIdxs...> /*idxs*/) {
      (op(get_at<tIdxs>(range)), ...);
    }(std::make_index_sequence<size>{});
  }
};

template<typename TOp>
inline constexpr auto for_each(TOp&& op) {
  return ForEachGenerator<TOp>{std::forward<TOp>(op)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_FOR_EACH_HPP
