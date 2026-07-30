// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_INFO_RESULT_HPP
#define INCLUDE_THESAUROS_UTILITY_INFO_RESULT_HPP

#include <cassert>
#include <stdexcept>
#include <utility>

namespace thes {
template<typename V, typename Info, Info Valid>
struct InfoResult {
  constexpr InfoResult(V value, Info info) : value_{std::move(value)}, info_{std::move(info)} {}

  [[nodiscard]] constexpr V operator*() const {
    assert(is_valid());
    return value_;
  }
  [[nodiscard]] constexpr V value_or(V def) const {
    return is_valid() ? value_ : def;
  }

  [[nodiscard]] constexpr bool is_valid() const {
    return info_ == Valid;
  }
  constexpr V valid_value() const {
    if (!is_valid()) {
      throw std::runtime_error("The value is invalid!");
    }
    return value_;
  }
  [[nodiscard]] constexpr V raw() const {
    return value_;
  }

private:
  V value_;
  Info info_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_INFO_RESULT_HPP
