// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_QUANTITY_CONVERT_HPP
#define INCLUDE_THESAUROS_QUANTITY_CONVERT_HPP

#include <chrono>

#include "thesauros/quantity/quantity.hpp"

namespace thes {
template<typename TRep, typename TPeriod>
inline constexpr auto duration_quantity(const std::chrono::duration<TRep, TPeriod>& d) {
  return Quantity<TRep, Unit<TPeriod, base_unit::second>>(d.count());
}

template<typename TRep, typename TPeriod>
inline constexpr auto quantity_duration(const Quantity<TRep, Unit<TPeriod, base_unit::second>>& d) {
  return std::chrono::duration<TRep, TPeriod>(d.count());
}
} // namespace thes

#endif // INCLUDE_THESAUROS_QUANTITY_CONVERT_HPP
