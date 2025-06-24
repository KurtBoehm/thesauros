// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_STATIC_APPLY_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_STATIC_APPLY_HPP

#include <cstddef>
#include <utility>

namespace thes::star {
template<std::size_t tSize>
constexpr decltype(auto) static_apply(auto f) {
  return [&]<std::size_t... tIdxs>(std::index_sequence<tIdxs...> /*seq*/) -> decltype(auto) {
    return f.template operator()<tIdxs...>();
  }(std::make_index_sequence<tSize>{});
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_DEFINITIONS_STATIC_APPLY_HPP
