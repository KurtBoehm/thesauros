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
#include "thesauros/types/value-tag.hpp"

namespace thes::star {
struct ToArrayGenerator : ConsumerGeneratorBase {
  template<typename R>
  THES_ALWAYS_INLINE constexpr auto operator()(R&& range) const {
    using Range = std::remove_cvref_t<R>;
    constexpr std::size_t size = star::size<Range>;

    if constexpr (size > 0) {
      return star::static_apply<size>([range = std::forward<R>(range)]<std::size_t... I> {
        using std::get;
        return std::array{get<I>(range)...};
      });
    } else {
      return std::array<star::Value<Range>, size>{};
    }
  }
};

inline constexpr ToArrayGenerator to_array{};

/**
 * Calls `op` in one of three ways (in the order in which they are checked), with `I` in `[0, N)`:
 * - `op.template operator()<I>()`.
 * - `op(index_tag<I>)`.
 * - `op()`.
 * The value type of the returned array is either `V` or, if it is `void` and `N` is non-zero, the
 * return type of `op`, assuming it returns the same type for each index; anything else is invalid.
 */
template<std::size_t N, typename V = void>
constexpr auto generate_array(auto op) {
  auto f = [&]<std::size_t I>(thes::IndexTag<I> i) {
    if constexpr (requires { op.template operator()<I>(); }) {
      return op.template operator()<I>();
    } else if constexpr (requires { op(i); }) {
      return op(i);
    } else if constexpr (requires { op(); }) {
      return op();
    } else {
      static_assert(false, "op is not callable with any supported signature");
    }
  };
  if constexpr (std::is_void_v<V>) {
    static_assert(N > 0, "The value type cannot be derived if the array is empty!");
    return static_apply<N>([&]<std::size_t... I> { return std::array{f(index_tag<I>)...}; });
  } else {
    return static_apply<N>([&]<std::size_t... I> { return std::array<V, N>{f(index_tag<I>)...}; });
  }
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_TO_ARRAY_HPP
