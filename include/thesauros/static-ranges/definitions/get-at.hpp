// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_GET_AT_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_GET_AT_HPP

#include <array>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes::star {
template<std::size_t tIndex, typename TRange>
concept HasGet = requires(TRange&& rng) { get<tIndex>(std::forward<TRange>(rng)); };

template<std::size_t tIndex, typename TRange>
requires(tIndex < std::tuple_size_v<std::decay_t<TRange>>)
THES_ALWAYS_INLINE inline constexpr decltype(auto) get_at(TRange&& r,
                                                          IndexTag<tIndex> /*tag*/ = {}) {
  using std::get;
  return get<tIndex>(std::forward<TRange>(r));
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_GET_AT_HPP
