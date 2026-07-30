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
#include "thesauros/static-ranges/definitions/static-apply.hpp"
#include "thesauros/static-ranges/definitions/type-traits.hpp"

namespace thes::star {
struct ToArrayGenerator : public ConsumerGeneratorBase {
  template<typename R>
  THES_ALWAYS_INLINE constexpr auto operator()(R&& range) const {
    using Range = std::decay_t<R>;
    constexpr std::size_t size = thes::star::size<Range>;

    if constexpr (size > 0) {
      return star::static_apply<size>([range = std::forward<R>(range)]<std::size_t... I>() {
        using std::get;
        return std::array{get<I>(range)...};
      });
    } else {
      return std::array<star::Value<Range>, size>{};
    }
  }
};

inline constexpr ToArrayGenerator to_array{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_TO_ARRAY_HPP
