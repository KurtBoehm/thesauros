// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_RANGES_AS_SIZED_HPP
#define INCLUDE_THESAUROS_RANGES_AS_SIZED_HPP

#include <ranges>
#include <vector>

namespace thes::ranges {
/**
 * Converts a range into a sized range by materializing it into an `std::vector` if it is not and
 * returning it unchanged if not.
 */
template<std::ranges::range Range>
inline decltype(auto) as_sized(Range&& range) {
  if constexpr (std::ranges::sized_range<Range>) {
    return std::forward<Range>(range);
  } else {
    return std::forward<Range>(range) |
           std::ranges::to<std::vector<std::ranges::range_value_t<Range>>>();
  }
}
} // namespace thes::ranges

#endif // INCLUDE_THESAUROS_RANGES_AS_SIZED_HPP
