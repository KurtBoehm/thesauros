// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_KIND_CONVERT_HPP
#define INCLUDE_THESAUROS_UTILITY_KIND_CONVERT_HPP

namespace thes {
template<class T>
constexpr T& as_reference(T& t) noexcept {
  return t;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_KIND_CONVERT_HPP
