// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_STORE_TYPE_HPP
#define INCLUDE_THESAUROS_UTILITY_STORE_TYPE_HPP

#include <memory>
#include <type_traits>

namespace thes {
template<typename T>
struct StoreTypeTrait {
  using Type = std::remove_reference_t<T>;
};
template<typename T>
struct StoreTypeTrait<T&> {
  using Type = std::add_pointer_t<T>;
};
template<typename T>
using StoreType = typename StoreTypeTrait<T>::Type;

template<typename T>
constexpr StoreType<T&&> to_stored(T&& value) noexcept {
  if constexpr (std::is_lvalue_reference_v<T&&>) {
    return std::addressof(value);
  } else {
    return std::forward<T>(value);
  }
}

template<typename R>
constexpr std::remove_reference_t<R>& from_stored(StoreType<R>& s) noexcept {
  if constexpr (std::is_lvalue_reference_v<R>) {
    return *s;
  } else {
    return s;
  }
}

template<typename R>
constexpr const std::remove_reference_t<R>& from_stored(const StoreType<R>& s) noexcept {
  if constexpr (std::is_lvalue_reference_v<R>) {
    return *s;
  } else {
    return s;
  }
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_STORE_TYPE_HPP
