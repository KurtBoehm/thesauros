// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_ALGORITHMS_STATIC_RANGES_POSITION_TO_INDEX_HPP
#define INCLUDE_THESAUROS_ALGORITHMS_STATIC_RANGES_POSITION_TO_INDEX_HPP

#include <concepts>
#include <cstddef>
#include <utility>

#include "thesauros/static-ranges/definitions/get-at.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/definitions/type-traits.hpp"

namespace thes::star {
template<typename Pos, typename Prods>
requires(std::same_as<star::Value<Pos>, star::Value<Prods>> &&
         star::size<Pos> + 1 == star::size<Prods>)
constexpr auto position_to_index(const Pos& pos, const Prods& incl_postfix_products) {
  constexpr auto size = star::size<Pos>;
  return [&]<std::size_t... I>(std::index_sequence<I...> /*idxs*/) {
    return (... + (star::get_at<I>(pos) * star::get_at<I + 1>(incl_postfix_products)));
  }(std::make_index_sequence<size>{});
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_ALGORITHMS_STATIC_RANGES_POSITION_TO_INDEX_HPP
