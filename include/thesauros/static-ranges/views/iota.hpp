// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_IOTA_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_IOTA_HPP

#include <concepts>
#include <cstddef>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/math/arithmetic.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes::star {
template<std::unsigned_integral TSize, TSize tBegin, TSize tEnd, TSize tStep>
struct IotaView {
  static constexpr TSize size = div_ceil(tEnd - tBegin, tStep);

  template<std::size_t tIndex>
  requires(tBegin + tIndex * tStep < tEnd)
  THES_ALWAYS_INLINE constexpr decltype(auto) get() const {
    return value_tag<TSize, tBegin + TSize{tIndex} * tStep>;
  }
};

template<std::size_t tBegin, std::size_t tEnd, std::size_t tStep = 1>
inline constexpr IotaView<std::size_t, tBegin, tEnd, tStep> iota{};
template<std::unsigned_integral TSize, TSize tBegin, TSize tEnd, TSize tStep = 1>
inline constexpr IotaView<TSize, tBegin, tEnd, tStep> typed_iota{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_IOTA_HPP
