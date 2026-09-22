// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_TYPES_TYPE_TAG_HPP
#define INCLUDE_THESAUROS_TYPES_TYPE_TAG_HPP

#include <type_traits>
#include <utility>

namespace thes {
template<typename T>
struct TypeTag {
  using Type = T;

  template<typename... Args>
  requires(std::is_constructible_v<T, Args...>)
  [[nodiscard]] constexpr T construct(Args&&... args) const {
    return T(std::forward<Args>(args)...);
  }
};

template<typename T>
inline constexpr TypeTag<T> type_tag{};

template<typename T>
inline constexpr bool is_type_tag = false;
template<typename T>
inline constexpr bool is_type_tag<TypeTag<T>> = true;
template<typename T>
concept AnyTypeTag = is_type_tag<T>;
} // namespace thes

#endif // INCLUDE_THESAUROS_TYPES_TYPE_TAG_HPP
