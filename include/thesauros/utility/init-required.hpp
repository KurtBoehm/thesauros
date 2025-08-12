// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_INIT_REQUIRED_HPP
#define INCLUDE_THESAUROS_UTILITY_INIT_REQUIRED_HPP

namespace thes {
// TODO Is there a better solution?
inline constexpr struct {
  template<typename T>
  operator T() const;
} init_required{};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_INIT_REQUIRED_HPP
