// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_ELEMENTS_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_ELEMENTS_HPP

#include <cstddef>
#include <utility>

#include "thesauros/static-ranges/definitions.hpp"
#include "thesauros/static-ranges/views/transform.hpp"

namespace thes::star {
template<std::size_t tIndex>
inline constexpr auto elements =
  transform([]<typename T>(T&& v) { return thes::star::get_at<tIndex>(std::forward<T>(v)); });
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_ELEMENTS_HPP
