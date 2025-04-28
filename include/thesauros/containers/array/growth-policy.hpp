#ifndef INCLUDE_THESAUROS_CONTAINERS_ARRAY_GROWTH_POLICY_HPP
#define INCLUDE_THESAUROS_CONTAINERS_ARRAY_GROWTH_POLICY_HPP

#include <cassert>
#include <cstddef>

#include "thesauros/math/bit.hpp"
#include "thesauros/types/numeric-info.hpp"

namespace thes {
struct DoublingGrowth {
  using Size = std::size_t;

  static constexpr Size new_allocation_size([[maybe_unused]] Size old_size,
                                            Size new_size_lower_bound) {
    assert(new_size_lower_bound > old_size);
    constexpr Size one = 1;

    const unsigned logarithm = bit_width(new_size_lower_bound);
    if (logarithm >= NumericInfo<Size>::digits) {
      return NumericInfo<Size>::max;
    }
    return one << logarithm;
  }
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_ARRAY_GROWTH_POLICY_HPP
