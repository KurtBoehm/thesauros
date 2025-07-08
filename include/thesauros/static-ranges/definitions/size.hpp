// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_SIZE_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_SIZE_HPP

#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/concepts/type-traits.hpp"

namespace thes::star {
template<typename TRange>
concept HasSize = CompleteType<std::tuple_size<std::decay_t<TRange>>>;
template<HasSize TRange>
inline constexpr std::size_t size = std::tuple_size_v<std::decay_t<TRange>>;
template<HasSize TRange>
inline constexpr bool is_empty = size<TRange> == 0;
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_SIZE_HPP
