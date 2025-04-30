// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_AS_EXPECTED_HPP
#define INCLUDE_THESAUROS_UTILITY_AS_EXPECTED_HPP

#include "tl/expected.hpp"

namespace thes {
inline constexpr tl::expected<void, int> as_expected(int ret) {
  using Out = tl::expected<void, int>;

  Out out{};
  if (ret != 0) {
    out = tl::unexpected(ret);
  }

  return out;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_AS_EXPECTED_HPP
