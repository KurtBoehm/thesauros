#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_MAP_VALUES_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_MAP_VALUES_HPP

#include <tuple>

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/get-at.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/value-sequence.hpp"

namespace thes::star {
template<typename TOp, typename... TInners>
requires(AutoValueSequence<size<TInners>...>::is_unique)
struct ValueMapView {
  TOp op;
  std::tuple<TInners...> inners;

  static constexpr std::size_t size = AutoValueSequence<::thes::star::size<TInners>...>::unique;

  template<std::size_t tIndex>
  constexpr decltype(auto) get() const {
    return std::apply([this](const auto&... ranges) { return op(get_at<tIndex>(ranges)...); },
                      inners);
  }
};

template<typename TOp>
struct ValueMapGenerator {
  TOp op;

  template<typename... TInners>
  constexpr auto operator()(TInners&&... inners) const {
    return ValueMapView<TOp, TInners...>{op, {std::forward<TInners>(inners)...}};
  }
};
template<typename TOp>
struct RangeGeneratorTrait<ValueMapGenerator<TOp>> : public std::true_type {};

template<typename TOp>
inline constexpr ValueMapGenerator<TOp> map_values(TOp&& op) {
  return {std::forward<TOp>(op)};
};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_MAP_VALUES_HPP
