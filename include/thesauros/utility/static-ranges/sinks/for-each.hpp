#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_FOR_EACH_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_FOR_EACH_HPP

#include <cstddef>
#include <utility>

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"

namespace thes::star {
template<typename TFun>
struct ForEachGenerator : public ConsumerGeneratorBase {
  TFun fun;

  explicit constexpr ForEachGenerator(TFun&& f) : fun(std::forward<TFun>(f)) {}

  template<typename TRange>
  constexpr void operator()(TRange&& range) const {
    constexpr std::size_t size = thes::star::size<TRange>;
    return [&]<std::size_t... tIdxs>(std::index_sequence<tIdxs...> /*idxs*/) {
      (fun(get_at<tIdxs>(range)), ...);
    }(std::make_index_sequence<size>{});
  }
};

template<typename TOp>
inline constexpr auto for_each(TOp&& op) {
  return ForEachGenerator<TOp>{std::forward<TOp>(op)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_FOR_EACH_HPP
