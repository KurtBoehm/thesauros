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
