// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_REVERSED_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_REVERSED_HPP

#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/get-at.hpp"
#include "thesauros/static-ranges/definitions/printable.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes::star {
template<typename TInner>
struct ReversedView {
  using Inner = std::decay_t<TInner>;
  static constexpr std::size_t size = thes::star::size<Inner>;
  static constexpr PrintableMarker printable{};

  TInner inner;

  template<std::size_t tIndex>
  THES_ALWAYS_INLINE constexpr auto get(IndexTag<tIndex> /*index*/) const {
    return get_at<size - tIndex - 1>(inner);
  }
};

struct ReversedGenerator : public RangeGeneratorBase {
  template<typename TRange>
  THES_ALWAYS_INLINE constexpr ReversedView<TRange> operator()(TRange&& range) const {
    return {std::forward<TRange>(range)};
  }
};

inline constexpr ReversedGenerator reversed{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_REVERSED_HPP
