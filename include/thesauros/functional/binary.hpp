// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_FUNCTIONAL_BINARY_HPP
#define INCLUDE_THESAUROS_FUNCTIONAL_BINARY_HPP

#include <algorithm>
#include <utility>

namespace thes {
struct Maximum {
  template<typename T1, typename T2>
  constexpr decltype(auto) operator()(T1&& v1, T2&& v2) const {
    return std::max(std::forward<T1>(v1), std::forward<T2>(v2));
  }
};
struct Minimum {
  template<typename T1, typename T2>
  constexpr decltype(auto) operator()(T1&& v1, T2&& v2) const {
    return std::min(std::forward<T1>(v1), std::forward<T2>(v2));
  }
};
} // namespace thes

#endif // INCLUDE_THESAUROS_FUNCTIONAL_BINARY_HPP
