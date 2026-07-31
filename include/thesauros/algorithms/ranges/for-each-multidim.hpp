// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_ALGORITHMS_RANGES_FOR_EACH_MULTIDIM_HPP
#define INCLUDE_THESAUROS_ALGORITHMS_RANGES_FOR_EACH_MULTIDIM_HPP

#include <cstddef>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/ranges/indices.hpp"
#include "thesauros/static-ranges/definitions/get-at.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/views/transform.hpp"
#include "thesauros/utility/static-map.hpp"

namespace thes {
template<typename Ranges, typename FixedAxes>
THES_ALWAYS_INLINE inline constexpr void multidim_for_each(const Ranges& ranges,
                                                           const FixedAxes& fixed_axes, auto&& f) {
  constexpr std::size_t size = star::size<Ranges>;

  auto impl = [&](auto& self, auto&&... vals) THES_ALWAYS_INLINE {
    constexpr auto index = sizeof...(vals);
    static_assert(index <= size);
    if constexpr (index == size) {
      f(std::move(vals)...);
    } else if constexpr (FixedAxes::contains(auto_tag<index>)) {
      self(self, std::move(vals)..., fixed_axes.get(auto_tag<index>));
    } else {
      for (auto&& value : star::get_at<index>(ranges)) {
        self(self, vals..., std::move(value));
      }
    }
  };
  impl(impl);
}

template<typename Ranges, typename F>
THES_ALWAYS_INLINE inline constexpr void multidim_for_each(const Ranges& ranges, F&& f) {
  multidim_for_each(ranges, StaticMap{}, std::forward<F>(f));
}

template<typename Sizes, typename FixedAxes, typename F>
THES_ALWAYS_INLINE inline constexpr void
multidim_for_each_size(const Sizes& sizes, const FixedAxes& fixed_axes, F&& f) {
  multidim_for_each(star::transform([](auto size) { return views::indices(size); })(sizes),
                    fixed_axes, std::forward<F>(f));
}
template<typename Sizes, typename F>
THES_ALWAYS_INLINE inline constexpr void multidim_for_each_size(const Sizes& sizes, F&& f) {
  multidim_for_each_size(sizes, StaticMap{}, std::forward<F>(f));
}
} // namespace thes

#endif // INCLUDE_THESAUROS_ALGORITHMS_RANGES_FOR_EACH_MULTIDIM_HPP
