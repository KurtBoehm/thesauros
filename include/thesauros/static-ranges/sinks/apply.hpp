#ifndef INCLUDE_THESAUROS_STATIC_RANGES_SINKS_APPLY_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_SINKS_APPLY_HPP

#include <cstddef>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"

namespace thes::star {
template<typename TFun>
struct ApplyGenerator : public ConsumerGeneratorBase {
  TFun fun;

  explicit constexpr ApplyGenerator(TFun&& f) : fun(std::forward<TFun>(f)) {}

  template<typename TRange>
  THES_ALWAYS_INLINE constexpr decltype(auto) operator()(TRange&& range) const {
    constexpr std::size_t size = thes::star::size<TRange>;
    return [&]<std::size_t... tIdxs>(std::index_sequence<tIdxs...> /*idxs*/)
             THES_ALWAYS_INLINE -> decltype(auto) {
               return fun(get_at<tIdxs>(range)...);
             }(std::make_index_sequence<size>{});
  }
};

template<typename TOp>
THES_ALWAYS_INLINE inline constexpr auto apply(TOp&& op) {
  return ApplyGenerator<TOp>{std::forward<TOp>(op)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_APPLY_HPP
