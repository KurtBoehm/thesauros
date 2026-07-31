// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_REVERSED_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_REVERSED_HPP

#include <cstddef>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/get-at.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/definitions/tuple-defs.hpp"

namespace thes::star {
template<typename Inner>
struct ReversedView {
  static constexpr std::size_t size = thes::star::size<Inner>;
  static constexpr TupleDefsMarker tuple_defs_marker{};

  Inner inner;

  template<std::size_t I>
  THES_ALWAYS_INLINE friend constexpr auto get(const ReversedView& self) {
    return get_at<size - I - 1>(self.inner);
  }
};

struct ReversedGenerator : public RangeGeneratorBase {
  template<typename Range>
  THES_ALWAYS_INLINE constexpr ReversedView<Range> operator()(Range&& range) const {
    return {std::forward<Range>(range)};
  }
};

inline constexpr ReversedGenerator reversed{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_REVERSED_HPP
