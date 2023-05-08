#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_IOTA_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_IOTA_HPP

#include <cstddef>

#include "thesauros/numerics/arithmetic.hpp"
#include "thesauros/utility/static-value.hpp"

namespace thes::star {
template<std::size_t tBegin, std::size_t tEnd, std::size_t tStep>
struct IotaView {
  static constexpr std::size_t size = div_ceil(tEnd - tBegin, tStep);

  template<std::size_t tIndex>
  requires(tBegin + tIndex * tStep < tEnd)
  constexpr decltype(auto) get() const {
    return static_auto<tBegin + tIndex * tStep>;
  }
};

template<std::size_t tBegin, std::size_t tEnd, std::size_t tStep = 1>
inline constexpr IotaView<tBegin, tEnd, tStep> iota{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_IOTA_HPP
