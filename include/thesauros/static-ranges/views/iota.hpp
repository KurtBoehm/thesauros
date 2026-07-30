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
#include "thesauros/static-ranges/definitions/tuple-defs.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes::star {
template<std::unsigned_integral S, S Begin, S End, S Step, bool TaggedValues>
struct IotaView {
  static constexpr S size = div_ceil(End - Begin, Step);
  static constexpr TupleDefsMarker tuple_defs_marker{};

  template<std::size_t I>
  requires(Begin + I * Step < End)
  THES_ALWAYS_INLINE friend constexpr decltype(auto) get(const IotaView& /*self*/) {
    static constexpr S value = Begin + (S{I} * Step);
    if constexpr (TaggedValues) {
      return value_tag<S, value>;
    } else {
      return value;
    }
  }
};

template<std::size_t Begin, std::size_t End, std::size_t Step = 1>
inline constexpr IotaView<std::size_t, Begin, End, Step, false> iota{};
template<std::unsigned_integral S, S Begin, S End, S Step = 1>
inline constexpr IotaView<S, Begin, End, Step, false> typed_iota{};

template<std::size_t Begin, std::size_t End, std::size_t Step = 1>
inline constexpr IotaView<std::size_t, Begin, End, Step, true> tagged_iota{};
template<std::unsigned_integral S, S Begin, S End, S Step = 1>
inline constexpr IotaView<S, Begin, End, Step, true> typed_tagged_iota{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_IOTA_HPP
