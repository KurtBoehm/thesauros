// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_ARRAY_INITIALIZATION_POLICY_HPP
#define INCLUDE_THESAUROS_CONTAINERS_ARRAY_INITIALIZATION_POLICY_HPP

#include <memory>

namespace thes {
struct ValueInit {
  template<typename T>
  static constexpr void initialize(T* begin, T* end) {
    std::uninitialized_value_construct(begin, end);
  }
};

struct DefaultInit {
  template<typename T>
  static constexpr void initialize(T* begin, T* end) {
    std::uninitialized_default_construct(begin, end);
  }
};

struct NoInit {
  template<typename T>
  static constexpr void initialize(T* /*begin*/, T* /*end*/) {}
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_ARRAY_INITIALIZATION_POLICY_HPP
