// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_CHECK_VARIANT_HPP
#define INCLUDE_THESAUROS_UTILITY_CHECK_VARIANT_HPP

#include <concepts>
#include <exception>
#include <utility>
#include <variant>

#include "thesauros/utility/fancy-visit.hpp"

namespace thes {
template<typename... Ts>
inline constexpr auto check_variant(std::variant<Ts...>&& variant, auto checker) {
  return fancy_filter_visit(
    [&]<typename T>(T&& val) {
      if constexpr (std::derived_from<decltype(checker(val)), std::exception>) {
        throw checker(val);
        return fancy_visitor_ignore;
      } else {
        return std::forward<T>(val);
      }
    },
    variant);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_CHECK_VARIANT_HPP
