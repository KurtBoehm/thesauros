#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_TRANSFORM_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_TRANSFORM_HPP

#include <array>
#include <cstddef>
#include <tuple>
#include <type_traits>

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/get-at.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/static-ranges/ranges/iota.hpp"
#include "thesauros/utility/static-ranges/sinks/apply.hpp"
#include "thesauros/utility/static-ranges/sinks/unique-value.hpp"
#include "thesauros/utility/tuple.hpp"

namespace thes::star {
template<typename TOp, typename... TInners>
requires(sizeof...(TInners) > 0 && star::has_unique_value(std::array{size<TInners>...}))
struct TransformView {
  TOp op;
  Tuple<TInners...> inners;

  explicit constexpr TransformView(TOp&& op, TInners&&... inners)
      : op(std::forward<TOp>(op)), inners(std::forward<TInners>(inners)...) {}

  static constexpr std::size_t size =
    (std::array{star::size<TInners>...} | star::unique_value).value();

  template<std::size_t tIndex>
  constexpr decltype(auto) get() const {
    return apply([this](const auto&... ranges) { return op(get_at<tIndex>(ranges)...); })(inners);
  }
};

template<typename TOp>
struct TransformGenerator : public RangeGeneratorBase {
  TOp op;

  explicit constexpr TransformGenerator(TOp&& op) : op(std::forward<TOp>(op)) {}

  template<typename... TInners>
  constexpr auto operator()(TInners&&... inners) const {
    return TransformView<TOp, TInners...>{TOp{op}, std::forward<TInners>(inners)...};
  }
};

template<typename TOp>
inline constexpr auto transform(TOp&& op) {
  return TransformGenerator<TOp>{std::forward<TOp>(op)};
};

template<typename TOp, typename... TInners>
requires(sizeof...(TInners) > 0)
inline constexpr auto transform(TOp&& op, TInners&&... inners) {
  return TransformView<TOp, TInners...>{std::forward<TOp>(op), std::forward<TInners>(inners)...};
}

template<std::size_t tSize, typename TOp>
inline constexpr auto index_transform(TOp&& op) {
  return TransformView<TOp, IotaView<0, tSize, 1>>{std::forward<TOp>(op), {}};
};
template<std::size_t tBegin, std::size_t tEnd, typename TOp>
inline constexpr auto index_transform(TOp&& op) {
  return TransformView<TOp, IotaView<tBegin, tEnd, 1>>{std::forward<TOp>(op), {}};
};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_TRANSFORM_HPP
