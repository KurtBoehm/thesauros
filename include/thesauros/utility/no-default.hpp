// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_NO_DEFAULT_HPP
#define INCLUDE_THESAUROS_UTILITY_NO_DEFAULT_HPP

#include <utility>

namespace thes {
template<typename T>
struct NoDef {
  constexpr NoDef(T&& val) : value{std::forward<T>(val)} {} // NOLINT
  template<typename... TArgs>
  requires(sizeof...(TArgs) > 0)
  constexpr NoDef(TArgs&&... args) : value{std::forward<TArgs>(args)...} {} // NOLINT

  constexpr operator const T&() const& noexcept { // NOLINT
    return value;
  }
  constexpr operator T&() & noexcept { // NOLINT
    return value;
  }
  constexpr operator const T&&() const&& noexcept { // NOLINT
    return std::move(value);
  }
  constexpr operator T&&() && noexcept { // NOLINT
    return std::move(value);
  }

  T value;
};
template<typename T>
NoDef(T&&) -> NoDef<T>;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_NO_DEFAULT_HPP
