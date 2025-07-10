// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_TYPE_TRAITS_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_TYPE_TRAITS_HPP

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "thesauros/types/type-sequence/type-sequence.hpp"

namespace thes::star {
template<std::size_t tIdx, typename TRange>
using Element = std::tuple_element_t<tIdx, std::decay_t<TRange>>;

template<typename TRange>
struct ValueSeqTrait {
  template<typename TIdxRange>
  struct Impl;

  template<std::size_t... tIdxs>
  struct Impl<std::index_sequence<tIdxs...>> {
    using Type = TypeSeq<Element<tIdxs, TRange>...>;
  };

  using Type = Impl<std::make_index_sequence<std::tuple_size_v<std::decay_t<TRange>>>>::Type;
};

template<typename TRange>
using ValueSeq = ValueSeqTrait<TRange>::Type;

namespace detail {
template<typename TRange>
concept HasValue = requires { typename TRange::Value; };
template<typename TRange>
concept HasTypeValue = requires { typename TRange::value_type; };
template<typename TRange>
concept HasElemType = ValueSeq<TRange>::is_unique;
} // namespace detail

template<typename TRange>
struct ValueTrait;

template<typename TRange>
requires detail::HasValue<TRange>
struct ValueTrait<TRange> {
  using Type = TRange::Value;
};
template<typename TRange>
requires(!detail::HasValue<TRange> && detail::HasTypeValue<TRange>)
struct ValueTrait<TRange> {
  using Type = TRange::value_type;
};
template<typename TRange>
requires(!detail::HasValue<TRange> && !detail::HasTypeValue<TRange> && detail::HasElemType<TRange>)
struct ValueTrait<TRange> {
  using Type = ValueSeq<TRange>::Unique;
};

template<typename TRange>
requires(detail::HasValue<TRange> || detail::HasTypeValue<TRange> || detail::HasElemType<TRange>)
using RawValue = ValueTrait<TRange>::Type;
template<typename TRange>
using Value = std::decay_t<RawValue<TRange>>;

template<typename TRange>
concept HasValue = requires { typename Value<TRange>; };
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_TYPE_TRAITS_HPP
