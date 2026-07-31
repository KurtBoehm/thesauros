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
template<AnyStaticRange R, IsPipeSink RGen>
THES_ALWAYS_INLINE inline constexpr decltype(auto) operator|(R&& range, RGen&& gen) {
  return std::forward<RGen>(gen)(std::forward<R>(range));
}

template<IsRangeGenerator RGen1, IsPipeSink RGen2>
THES_ALWAYS_INLINE inline constexpr decltype(auto) operator|(RGen1&& gen1, RGen2&& gen2) {
  return CombinedGenerator<RGen1, RGen2>{std::forward<RGen1>(gen1), std::forward<RGen2>(gen2)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_PIPING_HPP
