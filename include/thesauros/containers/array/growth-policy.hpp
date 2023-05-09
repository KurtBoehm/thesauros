#ifndef INCLUDE_THESAUROS_CONTAINERS_ARRAY_GROWTH_POLICY_HPP
#define INCLUDE_THESAUROS_CONTAINERS_ARRAY_GROWTH_POLICY_HPP

#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>

namespace thes::array {
struct DoublingGrowthPolicy {
  using Size = std::size_t;

  static constexpr Size new_allocation_size([[maybe_unused]] Size old_size,
                                            Size new_size_lower_bound) {
    assert(new_size_lower_bound > old_size);
    constexpr Size one = 1;

    const int logarithm = std::bit_width(new_size_lower_bound);
    if (logarithm >= std::numeric_limits<Size>::digits) {
      return std::numeric_limits<Size>::max();
    }
    return one << logarithm;
  }
};
} // namespace thes::array

#endif // INCLUDE_THESAUROS_CONTAINERS_ARRAY_GROWTH_POLICY_HPP
