#ifndef INCLUDE_THESAUROS_ALGORITHMS_STATIC_RANGES_multidim_for_each_HPP
#define INCLUDE_THESAUROS_ALGORITHMS_STATIC_RANGES_multidim_for_each_HPP

#include <iterator>

#include "thesauros/meta/static-value.hpp"
#include "thesauros/optimization.hpp"
#include "thesauros/utility/static-ranges/definitions/element-type.hpp"
#include "thesauros/utility/static-ranges/definitions/get-at.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"

namespace thes {
template<typename TRanges, typename TOp>
THES_ALWAYS_INLINE inline constexpr void multidim_for_each(TRanges&& ranges, TOp&& op) {
  using Range = star::ElementType<TRanges>;
  using Idx = typename std::iterator_traits<typename Range::const_iterator>::value_type;

  auto impl = [&op, &ranges](auto& self, auto index, auto&&... vals) THES_ALWAYS_INLINE {
    if constexpr (index == star::size<TRanges>) {
      op(std::move(vals)...);
    } else {
      for (auto&& value : star::get_at<index>(ranges)) {
        self(self, static_value<index + 1>, vals..., std::move(value));
      }
    }
  };
  impl(impl, static_value<Idx{0}>);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_ALGORITHMS_STATIC_RANGES_multidim_for_each_HPP
