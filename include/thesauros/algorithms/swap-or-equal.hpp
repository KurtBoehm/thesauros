// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_ALGORITHMS_SWAP_OR_EQUAL_HPP
#define INCLUDE_THESAUROS_ALGORITHMS_SWAP_OR_EQUAL_HPP

#include <cassert>
#include <type_traits>
#include <utility>

namespace thes {
template<typename T>
constexpr void swap_or_equal(std::remove_reference_t<T>& v1, std::remove_reference_t<T>& v2) {
  if constexpr (std::is_reference_v<T>) {
    assert(&v1 == &v2);
  } else {
    using std::swap;
    swap(v1, v2);
  }
};
} // namespace thes

#endif // INCLUDE_THESAUROS_ALGORITHMS_SWAP_OR_EQUAL_HPP
