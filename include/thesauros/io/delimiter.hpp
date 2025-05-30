// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_IO_DELIMITER_HPP
#define INCLUDE_THESAUROS_IO_DELIMITER_HPP

#include <string_view>
#include <utility>

namespace thes {
struct Delimiter {
  using Raw = std::string_view;

  explicit constexpr Delimiter(Raw str) : str_(std::forward<Raw>(str)) {}

  template<typename TIt>
  TIt output(TIt it, char sep) const {
    return output_impl(it, [&] { *it++ = sep; });
  }

  auto output(auto it) const {
    return output_impl(it, [] {});
  }

private:
  template<typename TIt>
  TIt output_impl(TIt it, auto op) const {
    if (first_) {
      first_ = false;
    } else {
      it = std::copy(str_.begin(), str_.end(), it);
      op();
    }
    return it;
  }

  Raw str_;
  mutable bool first_ = true;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_IO_DELIMITER_HPP
