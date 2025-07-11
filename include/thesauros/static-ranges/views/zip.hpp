// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_ZIP_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_ZIP_HPP

#include <array>
#include <cstddef>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/static-ranges/definitions/get-at.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/definitions/tuple-defs.hpp"
#include "thesauros/static-ranges/sinks/apply.hpp"
#include "thesauros/static-ranges/sinks/unique-value.hpp"
#include "thesauros/utility/tuple.hpp"

namespace thes::star {
template<typename... TRanges>
struct ZipView {
  static constexpr std::size_t size = *unique_value(std::array{thes::star::size<TRanges>...});
  static constexpr TupleDefsMarker tuple_defs_marker{};

  Tuple<TRanges...> ranges;

  template<std::size_t tIndex>
  THES_ALWAYS_INLINE friend constexpr auto get(const ZipView& self) {
    return apply([](auto&... inner) THES_ALWAYS_INLINE {
      return Tuple{thes::star::get_at<tIndex>(inner)...};
    })(self.ranges);
  }
};

template<typename... TRanges>
requires(sizeof...(TRanges) > 0)
THES_ALWAYS_INLINE inline constexpr auto zip(TRanges&&... ranges) {
  return ZipView<TRanges...>{Tuple{std::forward<TRanges>(ranges)...}};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_ZIP_HPP
