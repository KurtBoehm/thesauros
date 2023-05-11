#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_TRANSFORM_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_TRANSFORM_HPP

#include <tuple>

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/get-at.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/static-ranges/ranges/iota.hpp"
#include "thesauros/utility/value-sequence.hpp"

namespace thes::star {
template<typename TOp, typename... TInners>
requires(AutoSequence<size<TInners>...>::is_unique)
struct TransformView {
  TOp op;
  std::tuple<TInners...> inners;

  static constexpr std::size_t size = AutoSequence<thes::star::size<TInners>...>::unique;

  template<std::size_t tIndex>
  constexpr decltype(auto) get() const {
    return std::apply([this](const auto&... ranges) { return op(get_at<tIndex>(ranges)...); },
                      inners);
  }
};

template<typename TOp>
struct TransformGenerator {
  TOp op;

  template<typename... TInners>
  constexpr auto operator()(TInners&&... inners) const {
    return TransformView<TOp, TInners...>{op, {std::forward<TInners>(inners)...}};
  }
};
template<typename TOp>
struct RangeGeneratorTrait<TransformGenerator<TOp>> : public std::true_type {};

template<typename TOp>
inline constexpr TransformGenerator<TOp> transform(TOp&& op) {
  return {std::forward<TOp>(op)};
};

template<typename TOp, typename... TInners>
requires(sizeof...(TInners) > 0)
inline constexpr TransformView<TOp, TInners...> transform(TOp&& op, TInners&&... inners) {
  return {std::forward<TOp>(op), {std::forward<TInners>(inners)...}};
}

template<std::size_t tSize, typename TOp>
inline constexpr TransformView<TOp, IotaView<0, tSize, 1>> index_transform(TOp&& op) {
  return {std::forward<TOp>(op), {}};
};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_TRANSFORM_HPP
