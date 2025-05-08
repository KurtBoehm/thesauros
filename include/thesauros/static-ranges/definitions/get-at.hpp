// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_GET_AT_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_GET_AT_HPP

#include <array>
#include <concepts>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "thesauros/concepts/type-traits.hpp"
#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes::star {
namespace impl {
template<std::size_t tIndex, typename TRange>
concept HasMemberGet = requires(TRange&& rng) { std::forward<TRange>(rng).get(index_tag<tIndex>); };

template<std::size_t tIndex, typename TRange>
concept HasFreeGet = requires(TRange&& rng) { std::get<tIndex>(std::forward<TRange>(rng)); };
} // namespace impl

template<std::size_t tIndex, typename TRange>
struct GetAtTrait;

template<std::size_t tIndex, typename TRange>
requires impl::HasMemberGet<tIndex, TRange>
struct GetAtTrait<tIndex, TRange> {
  THES_ALWAYS_INLINE static constexpr decltype(auto) get_at(TRange&& range) {
    return std::forward<TRange>(range).get(index_tag<tIndex>);
  }
};

template<std::size_t tIndex, typename TRange>
requires(!impl::HasMemberGet<tIndex, TRange> && impl::HasFreeGet<tIndex, TRange>)
struct GetAtTrait<tIndex, TRange> {
  THES_ALWAYS_INLINE static constexpr decltype(auto) get_at(TRange&& range) {
    return std::get<tIndex>(std::forward<TRange>(range));
  }
};

template<std::size_t tIndex, typename TRange>
concept HasGetAt = CompleteType<GetAtTrait<tIndex, TRange>>;

template<std::size_t tIndex, typename TRange>
requires HasGetAt<tIndex, TRange>
THES_ALWAYS_INLINE inline constexpr decltype(auto) get_at(TRange&& r,
                                                          IndexTag<tIndex> /*tag*/ = {}) {
  return GetAtTrait<tIndex, TRange>::get_at(std::forward<TRange>(r));
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_GET_AT_HPP
