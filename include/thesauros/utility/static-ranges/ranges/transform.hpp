#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_TRANSFORM_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_TRANSFORM_HPP

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/get-at.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/static-ranges/definitions/type-traits.hpp"
#include "thesauros/utility/static-ranges/ranges/iota.hpp"
#include "thesauros/utility/static-ranges/sinks/apply.hpp"
#include "thesauros/utility/static-ranges/sinks/unique-value.hpp"
#include "thesauros/utility/tuple.hpp"

namespace thes::star {
namespace transform_impl {
template<typename TFun, typename TRet, typename... TArgRanges>
struct ValueBase {};

template<typename TFun, typename TRet, typename... TArgRanges>
requires(!std::is_void_v<TRet>)
struct ValueBase<TFun, TRet, TArgRanges...> {
  using Value = TRet;
};

template<typename TFun, typename TRet, typename... TArgRanges>
requires(std::is_void_v<TRet> && (... && HasValue<std::decay_t<TArgRanges>>))
struct ValueBase<TFun, TRet, TArgRanges...> {
  using Value =
    decltype(std::declval<const TFun&>()(std::declval<star::RawValue<std::decay_t<TArgRanges>>>()...));
};
} // namespace transform_impl

template<typename TFun, typename TRet, typename... TArgRanges>
requires(sizeof...(TArgRanges) > 0 && star::has_unique_value(std::array{size<TArgRanges>...}))
struct TransformView : public transform_impl::ValueBase<TFun, TRet, TArgRanges...> {
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

template<typename TFun, typename TRet = void>
struct TransformGenerator : public RangeGeneratorBase {
  TFun fun;

  explicit constexpr TransformGenerator(TFun&& f) : fun(std::forward<TFun>(f)) {}

  template<typename... TArgRanges>
  constexpr auto operator()(TArgRanges&&... ranges) const {
    return TransformView<TFun, TRet, TArgRanges...>{TFun{fun}, std::forward<TArgRanges>(ranges)...};
  }
};

template<typename TFun>
inline constexpr auto transform(TFun&& f) {
  return TransformGenerator<TFun>{std::forward<TFun>(f)};
};
template<typename TRet, typename TFun>
inline constexpr auto transform(TFun&& f) {
  return TransformGenerator<TFun, TRet>{std::forward<TFun>(f)};
};

template<typename TFun, typename... TArgRanges>
requires(sizeof...(TArgRanges) > 0)
inline constexpr auto transform(TFun&& f, TArgRanges&&... ranges) {
  return TransformView<TFun, void, TArgRanges...>{std::forward<TFun>(f),
                                                  std::forward<TArgRanges>(ranges)...};
}
template<typename TRet, typename TFun, typename... TArgRanges>
requires(sizeof...(TArgRanges) > 0)
inline constexpr auto transform(TFun&& f, TArgRanges&&... ranges) {
  return TransformView<TFun, TRet, TArgRanges...>{std::forward<TFun>(f),
                                                  std::forward<TArgRanges>(ranges)...};
}

template<std::size_t tSize, typename TFun>
inline constexpr auto index_transform(TFun&& f) {
  return TransformView<TFun, void, IotaView<0, tSize, 1>>{std::forward<TFun>(f), {}};
};
template<std::size_t tBegin, std::size_t tEnd, typename TFun>
inline constexpr auto index_transform(TFun&& f) {
  return TransformView<TFun, void, IotaView<tBegin, tEnd, 1>>{std::forward<TFun>(f), {}};
};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_TRANSFORM_HPP
