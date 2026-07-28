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
#include "thesauros/types/tuple.hpp"

namespace thes::star {
template<typename... Ranges>
struct ZipView {
  static constexpr std::size_t size = *unique_value(std::array{thes::star::size<Ranges>...});
  static constexpr TupleDefsMarker tuple_defs_marker{};

  Tuple<Ranges...> ranges;

  template<std::size_t I>
  THES_ALWAYS_INLINE friend constexpr auto get(const ZipView& self) {
    return apply([](auto&... inner) THES_ALWAYS_INLINE {
      return Tuple{thes::star::get_at<I>(inner)...};
    })(self.ranges);
  }
};

template<typename... Ranges>
requires(sizeof...(Ranges) > 0)
THES_ALWAYS_INLINE inline constexpr auto zip(Ranges&&... ranges) {
  return ZipView<Ranges...>{Tuple{std::forward<Ranges>(ranges)...}};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_ZIP_HPP
