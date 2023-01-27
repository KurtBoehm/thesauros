#ifndef INCLUDE_THESAUROS_ALGORITHMS_STATIC_RANGES_multidim_for_each_HPP
#define INCLUDE_THESAUROS_ALGORITHMS_STATIC_RANGES_multidim_for_each_HPP

#include <iterator>
#include <utility>

#include "thesauros/meta/static-value.hpp"
#include "thesauros/optimization.hpp"
#include "thesauros/ranges/iota.hpp"
#include "thesauros/utility/static-map.hpp"
#include "thesauros/utility/static-ranges/definitions/element-type.hpp"
#include "thesauros/utility/static-ranges/definitions/get-at.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/static-ranges/ranges/map-values.hpp"

namespace thes {
template<typename TRanges, typename TFixedAxes>
THES_ALWAYS_INLINE inline constexpr void
multidim_for_each(const TRanges& ranges, const TFixedAxes& fixed_axes, auto&& op) {
  constexpr std::size_t size = star::size<TRanges>;

  auto impl = [&fixed_axes, &op, &ranges](auto& self, auto&&... vals) THES_ALWAYS_INLINE {
    constexpr auto index = sizeof...(vals);
    static_assert(index <= size);
    if constexpr (index == size) {
      op(std::move(vals)...);
    } else if constexpr (TFixedAxes::template contains<index>) {
      self(self, std::move(vals)..., fixed_axes.template get<index>());
    } else {
      for (auto&& value : star::get_at<index>(ranges)) {
        self(self, vals..., std::move(value));
      }
    }
  };
  impl(impl);
}

template<typename TRanges, typename TOp>
THES_ALWAYS_INLINE inline constexpr void multidim_for_each(const TRanges& ranges, TOp&& op) {
  multidim_for_each(ranges, StaticMap{}, std::forward<TOp>(op));
}

template<typename TSizes, typename TFixedAxes, typename TOp>
THES_ALWAYS_INLINE inline constexpr void
multidim_for_each_size(const TSizes& sizes, const TFixedAxes& fixed_axes, TOp&& op) {
  multidim_for_each(sizes | star::map_values([](auto size) { return range(size); }), fixed_axes,
                    std::forward<TOp>(op));
}
template<typename TSizes, typename TOp>
THES_ALWAYS_INLINE inline constexpr void multidim_for_each_size(const TSizes& sizes, TOp&& op) {
  multidim_for_each_size(sizes, StaticMap{}, std::forward<TOp>(op));
}
} // namespace thes

#endif // INCLUDE_THESAUROS_ALGORITHMS_STATIC_RANGES_multidim_for_each_HPP
