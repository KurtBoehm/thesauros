#ifndef INCLUDE_THESAUROS_STATIC_RANGES_RANGES_TRANSFORM_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_RANGES_TRANSFORM_HPP

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/get-at.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/definitions/type-traits.hpp"
#include "thesauros/static-ranges/ranges/iota.hpp"
#include "thesauros/static-ranges/sinks/apply.hpp"
#include "thesauros/static-ranges/sinks/unique-value.hpp"
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
  using Value = decltype(std::declval<const TFun&>()(
    std::declval<star::RawValue<std::decay_t<TArgRanges>>>()...));
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
  THES_ALWAYS_INLINE constexpr decltype(auto) get() const {
    return apply([this](const auto&... ranges) THES_ALWAYS_INLINE -> decltype(auto) {
      return fun(get_at<tIndex>(ranges)...);
    })(range_tup);
  }
};

template<typename TFun, typename TRet = void>
struct TransformGenerator : public RangeGeneratorBase {
  TFun fun;

  explicit constexpr TransformGenerator(TFun&& f) : fun(std::forward<TFun>(f)) {}

  template<typename... TArgRanges>
  THES_ALWAYS_INLINE constexpr auto operator()(TArgRanges&&... ranges) const& {
    return TransformView<const TFun&, TRet, TArgRanges...>{fun,
                                                           std::forward<TArgRanges>(ranges)...};
  }
  template<typename... TArgRanges>
  THES_ALWAYS_INLINE constexpr auto operator()(TArgRanges&&... ranges) && {
    return TransformView<TFun, TRet, TArgRanges...>{std::forward<TFun>(fun),
                                                    std::forward<TArgRanges>(ranges)...};
  }
};

template<typename TFun>
THES_ALWAYS_INLINE inline constexpr auto transform(TFun&& f) {
  return TransformGenerator<TFun>{std::forward<TFun>(f)};
};
template<typename TRet, typename TFun>
THES_ALWAYS_INLINE inline constexpr auto transform(TFun&& f) {
  return TransformGenerator<TFun, TRet>{std::forward<TFun>(f)};
};

template<typename TFun, typename... TArgRanges>
requires(sizeof...(TArgRanges) > 0)
THES_ALWAYS_INLINE inline constexpr auto transform(TFun&& f, TArgRanges&&... ranges) {
  return TransformView<TFun, void, TArgRanges...>{std::forward<TFun>(f),
                                                  std::forward<TArgRanges>(ranges)...};
}
template<typename TRet, typename TFun, typename... TArgRanges>
requires(sizeof...(TArgRanges) > 0)
THES_ALWAYS_INLINE inline constexpr auto transform(TFun&& f, TArgRanges&&... ranges) {
  return TransformView<TFun, TRet, TArgRanges...>{std::forward<TFun>(f),
                                                  std::forward<TArgRanges>(ranges)...};
}

template<std::size_t tSize, typename TFun>
THES_ALWAYS_INLINE inline constexpr auto index_transform(TFun&& f) {
  using View = TransformView<TFun, void, IotaView<std::size_t, 0, tSize, 1>>;
  return View{std::forward<TFun>(f), {}};
};
template<std::size_t tBegin, std::size_t tEnd, typename TFun>
THES_ALWAYS_INLINE inline constexpr auto index_transform(TFun&& f) {
  using View = TransformView<TFun, void, IotaView<std::size_t, tBegin, tEnd, 1>>;
  return View{std::forward<TFun>(f), {}};
};
template<typename TSize, TSize tSize, typename TFun>
THES_ALWAYS_INLINE inline constexpr auto index_transform(TFun&& f) {
  using View = TransformView<TFun, void, IotaView<TSize, 0, tSize, 1>>;
  return View{std::forward<TFun>(f), {}};
};
template<typename TSize, TSize tBegin, TSize tEnd, typename TFun>
THES_ALWAYS_INLINE inline constexpr auto index_transform(TFun&& f) {
  using View = TransformView<TFun, void, IotaView<TSize, tBegin, tEnd, 1>>;
  return View{std::forward<TFun>(f), {}};
};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_RANGES_TRANSFORM_HPP
