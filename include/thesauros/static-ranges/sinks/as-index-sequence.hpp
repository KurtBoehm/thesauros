// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_SINKS_AS_INDEX_SEQUENCE_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_SINKS_AS_INDEX_SEQUENCE_HPP

#include <cstddef>
#include <utility>

#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/definitions/static-apply.hpp"

namespace thes::star {
template<AnyStaticRange auto Range>
inline constexpr auto as_index_sequence = static_apply<size<decltype(Range)>>(
  []<std::size_t... I>() { return std::index_sequence<get<I>(Range)...>{}; });

template<AnyStaticRange auto Range>
using AsIndexSequence = std::decay_t<decltype(as_index_sequence<Range>)>;
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_AS_INDEX_SEQUENCE_HPP
