// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_PIPING_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_PIPING_HPP

#include <utility>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/generators/combined.hpp"

namespace thes::star {
template<AnyStaticRange TRange, IsPipeSink TRangeGen>
THES_ALWAYS_INLINE inline constexpr decltype(auto) operator|(TRange&& range, TRangeGen&& gen) {
  return std::forward<TRangeGen>(gen)(std::forward<TRange>(range));
}

template<IsRangeGenerator TRangeGen1, IsPipeSink TRangeGen2>
THES_ALWAYS_INLINE inline constexpr decltype(auto) operator|(TRangeGen1&& gen1, TRangeGen2&& gen2) {
  return CombinedGenerator<TRangeGen1, TRangeGen2>{std::forward<TRangeGen1>(gen1),
                                                   std::forward<TRangeGen2>(gen2)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_PIPING_HPP
