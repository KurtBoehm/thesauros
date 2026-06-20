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
template<AnyStaticRange auto tRange>
inline constexpr auto as_index_sequence = static_apply<size<decltype(tRange)>>(
  []<std::size_t... tI>() { return std::index_sequence<get<tI>(tRange)...>{}; });

template<AnyStaticRange auto tRange>
using AsIndexSequence = std::decay_t<decltype(as_index_sequence<tRange>)>;
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_AS_INDEX_SEQUENCE_HPP
