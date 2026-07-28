// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_ALGORITHMS_STATIC_RANGES_INDEX_TO_POSITION_HPP
#define INCLUDE_THESAUROS_ALGORITHMS_STATIC_RANGES_INDEX_TO_POSITION_HPP

#include <array>
#include <cstddef>
#include <utility>

#include "thesauros/math/divmod.hpp"
#include "thesauros/static-ranges/definitions/get-at.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes::star {
template<typename Idx, typename S, std::size_t Dims>
constexpr auto index_to_position(Idx index, const std::array<S, Dims>& sizes) {
  auto impl = [&sizes]<typename... TArgs>(auto& self, Idx idx, auto end, TArgs&&... args) {
    static_assert(Dims > 0);

    if constexpr (Dims == 1) {
      static_assert(end == 1);
      static_assert(sizeof...(TArgs) == 0);
      return std::array{idx};
    }

    static_assert(end >= 2);
    const auto [div, mod] = divmod(idx, get_at<end - 1>(sizes));
    if constexpr (end == 2) {
      return std::array{div, mod, std::forward<TArgs>(args)...};
    } else {
      return self(self, div, index_tag<end - 1>, mod, std::forward<TArgs>(args)...);
    }
  };

  return impl(impl, index, index_tag<Dims>);
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_ALGORITHMS_STATIC_RANGES_INDEX_TO_POSITION_HPP
