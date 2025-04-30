// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_RANGES_CONSTANT_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_RANGES_CONSTANT_HPP

#include <cstddef>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"

namespace thes::star {
template<std::size_t tSize, typename T>
struct Constant {
  using Value = T;

  T value;

  static constexpr std::size_t size = tSize;

  template<std::size_t tIndex>
  requires(tIndex < tSize)
  THES_ALWAYS_INLINE constexpr auto get() const {
    return value;
  }
};

template<std::size_t tSize, typename T>
THES_ALWAYS_INLINE inline constexpr auto constant(T&& value) {
  return Constant<tSize, T>{std::forward<T>(value)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_RANGES_CONSTANT_HPP
