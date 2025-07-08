// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_TUPLE_DEFS_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_TUPLE_DEFS_HPP

#include <cstddef>
#include <tuple>

#include "thesauros/concepts/type-traits.hpp"

namespace thes::star {
struct TupleDefsMarker {};
template<typename T>
concept HasTupleDefsMarker = requires {
  { T::tuple_defs_marker } -> DecayedSameAs<TupleDefsMarker>;
};
} // namespace thes::star

template<thes::star::HasTupleDefsMarker TRange>
struct std::tuple_size<TRange> {
  static constexpr std::size_t value = TRange::size;
};

template<std::size_t tIdx, thes::star::HasTupleDefsMarker TRange>
requires(requires { typename TRange::Value; })
struct std::tuple_element<tIdx, TRange> {
  using type = TRange::Value;
};
template<std::size_t tIdx, thes::star::HasTupleDefsMarker TRange>
requires(!requires { typename TRange::Value; })
struct std::tuple_element<tIdx, TRange> {
  using type = decltype(get<tIdx>(std::declval<const TRange&>()));
};

#endif // INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_TUPLE_DEFS_HPP
