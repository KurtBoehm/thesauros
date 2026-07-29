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
template<typename Rep, typename Period>
constexpr auto duration_quantity(const std::chrono::duration<Rep, Period>& d) {
  return Quantity<Rep, Unit<Period, unit::base::second>>(d.count());
}

template<typename Rep, typename Period>
constexpr auto quantity_duration(const Quantity<Rep, Unit<Period, unit::base::second>>& d) {
  return std::chrono::duration<Rep, Period>(d.count());
}
} // namespace thes

#endif // INCLUDE_THESAUROS_QUANTITY_CONVERT_HPP
