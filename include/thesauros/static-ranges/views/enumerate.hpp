// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_ENUMERATE_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_ENUMERATE_HPP

#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/definitions/tuple-defs.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes::star {
template<typename TSize, typename TInner>
struct EnumerateView {
  using Inner = std::decay_t<TInner>;
  static constexpr std::size_t size = thes::star::size<Inner>;
  static constexpr TupleDefsMarker tuple_defs_marker{};

  TInner inner;

  template<std::size_t tIndex>
  THES_ALWAYS_INLINE friend constexpr std::pair<ValueTag<TSize, tIndex>,
                                                decltype(get_at<tIndex>(inner))>
  get(const EnumerateView& self) {
    return {value_tag<TSize, tIndex>, get_at<tIndex>(self.inner)};
  }
};

template<typename TSize>
struct EnumerateGenerator : public RangeGeneratorBase {
  template<typename TRange>
  THES_ALWAYS_INLINE constexpr EnumerateView<TSize, TRange> operator()(TRange&& range) const {
    return {std::forward<TRange>(range)};
  }
};

template<typename TSize = std::size_t>
inline constexpr EnumerateGenerator<TSize> enumerate{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_ENUMERATE_HPP
