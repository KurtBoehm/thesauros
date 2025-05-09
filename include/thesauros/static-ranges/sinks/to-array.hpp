// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_SINKS_TO_ARRAY_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_SINKS_TO_ARRAY_HPP

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/definitions/type-traits.hpp"
#include "thesauros/static-ranges/sinks/to-container.hpp"

namespace thes::star {
struct ToArrayGenerator : public ConsumerGeneratorBase {
  template<typename TRange>
  THES_ALWAYS_INLINE constexpr auto operator()(TRange&& range) const {
    using Range = std::decay_t<TRange>;
    using Value = star::Value<Range>;
    constexpr std::size_t size = thes::star::size<Range>;

    return to_container<std::array<Value, size>>(std::forward<TRange>(range));
  }
};

inline constexpr ToArrayGenerator to_array{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_TO_ARRAY_HPP
