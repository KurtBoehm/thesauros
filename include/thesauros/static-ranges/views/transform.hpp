// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_TRANSFORM_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_TRANSFORM_HPP

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/get-at.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/definitions/tuple-defs.hpp"
#include "thesauros/static-ranges/definitions/type-traits.hpp"
#include "thesauros/static-ranges/sinks/apply.hpp"
#include "thesauros/static-ranges/sinks/unique-value.hpp"
#include "thesauros/static-ranges/views/iota.hpp"
#include "thesauros/types/tuple.hpp"

namespace thes::star {
namespace detail::transform {
template<typename Fun, typename Ret, typename... ArgRanges>
struct ValueBase {};

template<typename Fun, typename Ret, typename... ArgRanges>
requires(!std::is_void_v<Ret>)
struct ValueBase<Fun, Ret, ArgRanges...> {
  using Value = Ret;
};

template<typename Fun, typename Ret, typename... ArgRanges>
requires(std::is_void_v<Ret> && (... && HasValue<std::decay_t<ArgRanges>>))
struct ValueBase<Fun, Ret, ArgRanges...> {
  using Value = decltype(std::declval<const Fun&>()(
    std::declval<star::RawValue<std::decay_t<ArgRanges>>>()...));
};
} // namespace detail::transform

template<typename Fun, typename Ret, typename... ArgRanges>
requires(sizeof...(ArgRanges) > 0 && star::has_unique_value(std::array{size<ArgRanges>...}))
struct TransformView : public detail::transform::ValueBase<Fun, Ret, ArgRanges...> {
  static constexpr std::size_t size =
    star::unique_value(std::array{star::size<ArgRanges>...}).value();
  static constexpr TupleDefsMarker tuple_defs_marker{};

  Fun fun;
  Tuple<ArgRanges...> range_tup;

  explicit constexpr TransformView(Fun&& f, ArgRanges&&... ranges)
      : fun(std::forward<Fun>(f)), range_tup(std::forward<ArgRanges>(ranges)...) {}

  template<std::size_t I>
  THES_ALWAYS_INLINE friend constexpr decltype(auto) get(const TransformView& self) {
    return apply([&self](const auto&... ranges) THES_ALWAYS_INLINE -> decltype(auto) {
      return self.fun(get_at<I>(ranges)...);
    })(self.range_tup);
  }
};

template<typename Fun, typename Ret = void>
struct TransformGenerator : public RangeGeneratorBase {
  Fun fun;

  explicit constexpr TransformGenerator(Fun&& f) : fun(std::forward<Fun>(f)) {}

  template<typename... ArgRanges>
  THES_ALWAYS_INLINE constexpr auto operator()(ArgRanges&&... ranges) const& {
    return TransformView<const Fun&, Ret, ArgRanges...>{fun, std::forward<ArgRanges>(ranges)...};
  }
  template<typename... ArgRanges>
  THES_ALWAYS_INLINE constexpr auto operator()(ArgRanges&&... ranges) && {
    return TransformView<Fun, Ret, ArgRanges...>{std::forward<Fun>(fun),
                                                 std::forward<ArgRanges>(ranges)...};
  }
};

template<typename Fun>
THES_ALWAYS_INLINE inline constexpr auto transform(Fun&& f) {
  return TransformGenerator<Fun>{std::forward<Fun>(f)};
}
template<typename Ret, typename Fun>
THES_ALWAYS_INLINE inline constexpr auto transform(Fun&& f) {
  return TransformGenerator<Fun, Ret>{std::forward<Fun>(f)};
}

template<typename Fun, typename... ArgRanges>
requires(sizeof...(ArgRanges) > 0)
THES_ALWAYS_INLINE inline constexpr auto transform(Fun&& f, ArgRanges&&... ranges) {
  return TransformView<Fun, void, ArgRanges...>{std::forward<Fun>(f),
                                                std::forward<ArgRanges>(ranges)...};
}
template<typename Ret, typename Fun, typename... ArgRanges>
requires(sizeof...(ArgRanges) > 0)
THES_ALWAYS_INLINE inline constexpr auto transform(Fun&& f, ArgRanges&&... ranges) {
  return TransformView<Fun, Ret, ArgRanges...>{std::forward<Fun>(f),
                                               std::forward<ArgRanges>(ranges)...};
}

template<std::size_t End, typename Fun>
THES_ALWAYS_INLINE inline constexpr auto index_transform(Fun&& f) {
  using View = TransformView<Fun, void, IotaView<std::size_t, 0, End, 1>>;
  return View{std::forward<Fun>(f), {}};
}
template<std::size_t Begin, std::size_t End, typename Fun>
THES_ALWAYS_INLINE inline constexpr auto index_transform(Fun&& f) {
  using View = TransformView<Fun, void, IotaView<std::size_t, Begin, End, 1>>;
  return View{std::forward<Fun>(f), {}};
}
template<typename Size, Size End, typename Fun>
THES_ALWAYS_INLINE inline constexpr auto index_transform(Fun&& f) {
  using View = TransformView<Fun, void, IotaView<Size, 0, End, 1>>;
  return View{std::forward<Fun>(f), {}};
}
template<typename Size, Size Begin, Size End, typename Fun>
THES_ALWAYS_INLINE inline constexpr auto index_transform(Fun&& f) {
  using View = TransformView<Fun, void, IotaView<Size, Begin, End, 1>>;
  return View{std::forward<Fun>(f), {}};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_TRANSFORM_HPP
