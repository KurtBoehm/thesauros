// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_TYPE_SEQ_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_TYPE_SEQ_HPP

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/types/type-sequence/type-sequence.hpp"

namespace thes {
/** The element types of the static range `Range`, in order, at the indices given by `IdxSeq`. */
template<star::AnyStaticRange Range, typename IdxSeq>
struct StaticRangeTypeSeqTrait;
template<star::AnyStaticRange Range, std::size_t... Idxs>
struct StaticRangeTypeSeqTrait<Range, std::index_sequence<Idxs...>> {
  using Type = TypeSeq<std::tuple_element_t<Idxs, std::remove_cvref_t<Range>>...>;
};

/** The element types of the static range `Range`, in order, as a `TypeSeq`. */
template<star::AnyStaticRange Range>
using StaticRangeTypeSeq =
  StaticRangeTypeSeqTrait<Range, std::make_index_sequence<star::size<Range>>>::Type;

/** Returns `StaticRangeTypeSeq<Range>{}`, the element types of `range` as a `TypeSeq`. */
template<star::AnyStaticRange Range>
constexpr StaticRangeTypeSeq<Range> static_range_type_seq(const Range& /*range*/) {
  return {};
}
} // namespace thes

#endif // INCLUDE_THESAUROS_STATIC_RANGES_TYPE_SEQ_HPP
