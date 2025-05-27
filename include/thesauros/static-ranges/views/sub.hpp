// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_SUB_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_SUB_HPP

#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/printable.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes::star {
template<typename TInner, std::size_t tBegin, std::size_t tEnd>
requires(tBegin <= tEnd && tEnd <= thes::star::size<TInner>)
struct SubView {
  using Inner = std::decay_t<TInner>;
  static constexpr std::size_t size = tEnd - tBegin;
  static constexpr PrintableMarker printable{};

  TInner inner;

  template<std::size_t tIndex>
  THES_ALWAYS_INLINE constexpr decltype(auto) get(IndexTag<tIndex> /*index*/) const {
    return get_at<tIndex + tBegin>(inner);
  }
};

template<std::size_t tBegin, std::size_t tEnd>
struct SubGenerator : public RangeGeneratorBase {
  template<typename TRange>
  THES_ALWAYS_INLINE constexpr SubView<TRange, tBegin, tEnd> operator()(TRange&& range) const {
    return {std::forward<TRange>(range)};
  }
};

template<std::size_t tBegin, std::size_t tEnd>
inline constexpr SubGenerator<tBegin, tEnd> sub{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_SUB_HPP
