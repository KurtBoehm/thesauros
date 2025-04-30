// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_RANGES_CONCEPTS_HPP
#define INCLUDE_THESAUROS_RANGES_CONCEPTS_HPP

#include <iterator>

namespace thes {
template<typename T>
concept AnyRange = requires(const T& r) {
  std::begin(r);
  std::end(r);
};
template<typename T>
concept MapRange = AnyRange<T> && requires(const T& r) {
  typename T::key_type;
  typename T::mapped_type;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_RANGES_CONCEPTS_HPP
