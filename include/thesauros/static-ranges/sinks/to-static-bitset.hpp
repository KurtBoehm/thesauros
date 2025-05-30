// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_SINKS_TO_STATIC_BITSET_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_SINKS_TO_STATIC_BITSET_HPP

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/containers/bitset/static.hpp"
#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/definitions/type-traits.hpp"
#include "thesauros/static-ranges/sinks/to-container.hpp"

namespace thes::star {
struct ToStaticBitsetGenerator : public ConsumerGeneratorBase {
  template<typename TRange>
  constexpr auto operator()(TRange&& range) const {
    using Range = std::decay_t<TRange>;
    static_assert(std::same_as<Value<TRange>, bool>);
    constexpr std::size_t size = thes::star::size<Range>;

    return to_container<StaticBitset<size>>(std::forward<TRange>(range));
  }
};

inline constexpr ToStaticBitsetGenerator to_static_bitset{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_TO_STATIC_BITSET_HPP
