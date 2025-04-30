// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_SIZE_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_SIZE_HPP

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/concepts/type-traits.hpp"

namespace thes::star {
namespace impl {
template<typename TRange>
concept HasMemberSize = requires {
  { TRange::size } -> std::convertible_to<std::size_t>;
};

template<typename TRange>
concept HasTupleSize = CompleteType<std::tuple_size<TRange>>;
} // namespace impl

template<typename TRange>
struct SizeTrait;

template<typename TRange>
requires impl::HasMemberSize<TRange>
struct SizeTrait<TRange> {
  static constexpr std::size_t value = TRange::size;
};
template<typename TRange>
requires(!impl::HasMemberSize<TRange> && impl::HasTupleSize<TRange>)
struct SizeTrait<TRange> {
  static constexpr std::size_t value = std::tuple_size_v<TRange>;
};

template<typename TRange>
concept HasSize = CompleteType<SizeTrait<std::decay_t<TRange>>>;
template<HasSize TRange>
inline constexpr std::size_t size = SizeTrait<std::decay_t<TRange>>::value;
template<HasSize TRange>
inline constexpr bool is_empty = size<TRange> == 0;
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_SIZE_HPP
