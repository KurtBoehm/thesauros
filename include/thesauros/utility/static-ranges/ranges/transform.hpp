#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_TRANSFORM_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_TRANSFORM_HPP

#include <array>
#include <cstddef>

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/get-at.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/static-ranges/ranges/iota.hpp"
#include "thesauros/utility/static-ranges/sinks/apply.hpp"
#include "thesauros/utility/static-ranges/sinks/unique-value.hpp"
#include "thesauros/utility/tuple.hpp"

namespace thes::star {
template<typename TFun, typename... TArgRanges>
requires(sizeof...(TArgRanges) > 0 && star::has_unique_value(std::array{size<TArgRanges>...}))
struct TransformView {
  TFun fun;
  Tuple<TArgRanges...> range_tup;

  explicit constexpr TransformView(TFun&& f, TArgRanges&&... ranges)
      : fun(std::forward<TFun>(f)), range_tup(std::forward<TArgRanges>(ranges)...) {}

  static constexpr std::size_t size =
    star::unique_value(std::array{star::size<TArgRanges>...}).value();

  template<std::size_t tIndex>
  constexpr decltype(auto) get() const {
    return apply([this](const auto&... ranges) { return fun(get_at<tIndex>(ranges)...); })(
      range_tup);
  }
};

template<typename TFun>
struct TransformGenerator : public RangeGeneratorBase {
  TFun fun;

  explicit constexpr TransformGenerator(TFun&& f) : fun(std::forward<TFun>(f)) {}

  template<typename... TArgRanges>
  constexpr auto operator()(TArgRanges&&... ranges) const {
    return TransformView<TFun, TArgRanges...>{TFun{fun}, std::forward<TArgRanges>(ranges)...};
  }
};

template<typename TFun>
inline constexpr auto transform(TFun&& f) {
  return TransformGenerator<TFun>{std::forward<TFun>(f)};
};

template<typename TFun, typename... TArgRanges>
requires(sizeof...(TArgRanges) > 0)
inline constexpr auto transform(TFun&& f, TArgRanges&&... ranges) {
  return TransformView<TFun, TArgRanges...>{std::forward<TFun>(f),
                                            std::forward<TArgRanges>(ranges)...};
}

template<std::size_t tSize, typename TFun>
inline constexpr auto index_transform(TFun&& f) {
  return TransformView<TFun, IotaView<0, tSize, 1>>{std::forward<TFun>(f), {}};
};
template<std::size_t tBegin, std::size_t tEnd, typename TFun>
inline constexpr auto index_transform(TFun&& f) {
  return TransformView<TFun, IotaView<tBegin, tEnd, 1>>{std::forward<TFun>(f), {}};
};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_TRANSFORM_HPP
