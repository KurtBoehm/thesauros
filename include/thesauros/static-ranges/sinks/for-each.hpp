// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_SINKS_FOR_EACH_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_SINKS_FOR_EACH_HPP

#include <cstddef>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"

namespace thes::star {
template<typename TFun>
struct ForEachGenerator : public ConsumerGeneratorBase {
  TFun fun;

  explicit constexpr ForEachGenerator(TFun&& f) : fun(std::forward<TFun>(f)) {}

  template<typename TRange>
  THES_ALWAYS_INLINE constexpr void operator()(TRange&& range) const {
    constexpr std::size_t size = thes::star::size<TRange>;
    return [&]<std::size_t... tIdxs>(std::index_sequence<tIdxs...> /*idxs*/) THES_ALWAYS_INLINE {
      (fun(get_at<tIdxs>(range)), ...);
    }(std::make_index_sequence<size>{});
  }
};

template<typename TOp>
THES_ALWAYS_INLINE inline constexpr auto for_each(TOp&& op) {
  return ForEachGenerator<TOp>{std::forward<TOp>(op)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_FOR_EACH_HPP
