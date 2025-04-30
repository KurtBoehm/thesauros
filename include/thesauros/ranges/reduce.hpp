// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_RANGES_REDUCE_HPP
#define INCLUDE_THESAUROS_RANGES_REDUCE_HPP

#include <iterator>
#include <numeric>

namespace thes {
template<typename TRange, typename T, typename TBinOp>
constexpr T reduce(TRange&& range, T init, TBinOp op) {
  return std::reduce(std::begin(range), std::end(range), init, op);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_RANGES_REDUCE_HPP
