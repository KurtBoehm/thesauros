// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_ENUMERATE_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_ENUMERATE_HPP

#include <cstddef>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/definitions/tuple-defs.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes::star {
template<typename S, typename Inner>
struct EnumerateView {
  static constexpr std::size_t size = thes::star::size<Inner>;
  static constexpr TupleDefsMarker tuple_defs_marker{};

  Inner inner;

  template<std::size_t I>
  THES_ALWAYS_INLINE friend constexpr std::pair<ValueTag<S, I>, decltype(get_at<I>(inner))>
  get(const EnumerateView& self) {
    return {value_tag<S, I>, get_at<I>(self.inner)};
  }
};

template<typename S>
struct EnumerateGenerator : public RangeGeneratorBase {
  template<typename Range>
  THES_ALWAYS_INLINE constexpr EnumerateView<S, Range> operator()(Range&& range) const {
    return {std::forward<Range>(range)};
  }
};

template<typename S = std::size_t>
inline constexpr EnumerateGenerator<S> enumerate{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_ENUMERATE_HPP
