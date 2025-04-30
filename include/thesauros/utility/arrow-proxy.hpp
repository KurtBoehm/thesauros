// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_ARROW_PROXY_HPP
#define INCLUDE_THESAUROS_UTILITY_ARROW_PROXY_HPP

#include <utility>

namespace thes {
template<typename T>
struct ArrowProxy {
  T value;

  T* operator->() {
    return &value;
  }
};

template<typename T, typename TOut>
struct ArrowCreator;

template<typename T>
struct ArrowCreator<T, T*> {
  static constexpr T* create(T& t) {
    return &t;
  }
};

template<typename T>
struct ArrowCreator<T, const T*> {
  static constexpr const T* create(const T& t) {
    return &t;
  }
};

template<typename T>
struct ArrowCreator<T, ArrowProxy<T>> {
  static constexpr ArrowProxy<T> create(T&& t) {
    return ArrowProxy<T>{std::forward<T>(t)};
  }
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_ARROW_PROXY_HPP
