// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_IO_DELIMITER_HPP
#define INCLUDE_THESAUROS_IO_DELIMITER_HPP

#include <algorithm>
#include <string_view>

namespace thes {
/**
 * Writes a separator before every element but the first, so that a sequence can be emitted without
 * a leading or trailing separator. Each call returns the iterator past what it wrote, which the
 * caller has to keep using.
 */
struct Delimiter {
  using Raw = std::string_view;

  explicit constexpr Delimiter(Raw str) : str_(str) {}

  /** Writes the separator, followed by `sep`, and returns the iterator past both. */
  template<typename TIt>
  constexpr TIt output(TIt it, char sep) const {
    return output_impl(it, [sep](TIt cur) {
      *cur++ = sep;
      return cur;
    });
  }

  /** Writes the separator and returns the iterator past it. */
  template<typename TIt>
  constexpr TIt output(TIt it) const {
    return output_impl(it, [](TIt cur) { return cur; });
  }

private:
  template<typename TIt>
  constexpr TIt output_impl(TIt it, auto op) const {
    if (first_) {
      first_ = false;
      return it;
    }
    return op(std::copy(str_.begin(), str_.end(), it));
  }

  Raw str_;
  mutable bool first_ = true;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_IO_DELIMITER_HPP
