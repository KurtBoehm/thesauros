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
template<std::size_t I, typename Range>
using Element = std::tuple_element_t<I, std::decay_t<Range>>;

template<typename Range>
struct ValueSeqTrait {
  template<typename IdxRange>
  struct Impl;

  template<std::size_t... I>
  struct Impl<std::index_sequence<I...>> {
    using Type = TypeSeq<Element<I, Range>...>;
  };

  using Type = Impl<std::make_index_sequence<std::tuple_size_v<std::decay_t<Range>>>>::Type;
};

template<typename Range>
using ValueSeq = ValueSeqTrait<Range>::Type;

namespace detail {
template<typename Range>
concept HasValue = requires { typename Range::Value; };
template<typename Range>
concept HasTypeValue = requires { typename Range::value_type; };
template<typename Range>
concept HasElemType = ValueSeq<Range>::is_unique;
} // namespace detail

template<typename Range>
struct ValueTrait;

template<typename Range>
requires detail::HasValue<Range>
struct ValueTrait<Range> {
  using Type = Range::Value;
};
template<typename Range>
requires(!detail::HasValue<Range> && detail::HasTypeValue<Range>)
struct ValueTrait<Range> {
  using Type = Range::value_type;
};
template<typename Range>
requires(!detail::HasValue<Range> && !detail::HasTypeValue<Range> && detail::HasElemType<Range>)
struct ValueTrait<Range> {
  using Type = ValueSeq<Range>::Unique;
};

template<typename Range>
requires(detail::HasValue<Range> || detail::HasTypeValue<Range> || detail::HasElemType<Range>)
using RawValue = ValueTrait<Range>::Type;
template<typename Range>
using Value = std::decay_t<RawValue<Range>>;

template<typename Range>
concept HasValue = requires { typename Value<Range>; };
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_TYPE_TRAITS_HPP
