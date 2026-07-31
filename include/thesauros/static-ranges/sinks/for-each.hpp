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
template<typename F>
struct ForEachGenerator : public ConsumerGeneratorBase {
  F fun;

  explicit constexpr ForEachGenerator(F&& f) : fun(std::forward<F>(f)) {}

  template<typename Range>
  THES_ALWAYS_INLINE constexpr void operator()(Range&& range) const {
    constexpr std::size_t size = thes::star::size<Range>;
    return [&]<std::size_t... I>(std::index_sequence<I...> /*idxs*/)
             THES_ALWAYS_INLINE { (fun(get_at<I>(range)), ...); }(std::make_index_sequence<size>{});
  }
};

template<typename F>
THES_ALWAYS_INLINE inline constexpr auto for_each(F&& op) {
  return ForEachGenerator<F>{std::forward<F>(op)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_FOR_EACH_HPP
