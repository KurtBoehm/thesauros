// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_MATH_IS_CLOSE_HPP
#define INCLUDE_THESAUROS_MATH_IS_CLOSE_HPP

#include <concepts>

#include "thesauros/string/static-string.hpp"
#include "thesauros/types/value-tag.hpp"
#include "thesauros/utility/static-map.hpp"

namespace thes {
template<std::floating_point T, typename... TArgs>
inline constexpr bool is_close(const T a, const T b, TArgs&&... kwargs) {
  using namespace literals;

  const StaticMap kwargs_map{std::forward<TArgs>(kwargs)...};
  static_assert(decltype(kwargs_map)::template only_keys<"rel_tol"_sstr, "abs_tol"_sstr>);
  const T rel_tol = kwargs_map.get(auto_tag<"rel_tol"_sstr>, T(1e-9));
  const T abs_tol = kwargs_map.get(auto_tag<"abs_tol"_sstr>, T(0.0));

  return std::abs(a - b) <= std::max(rel_tol * std::max(std::abs(a), std::abs(b)), abs_tol);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_MATH_IS_CLOSE_HPP
