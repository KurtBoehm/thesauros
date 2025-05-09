// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_SINKS_TO_CONTAINER_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_SINKS_TO_CONTAINER_HPP

#include <cstddef>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"

namespace thes::star {
template<typename TContainer>
struct ToContainerGenerator : public ConsumerGeneratorBase {
  template<typename TRange>
  THES_ALWAYS_INLINE constexpr auto operator()(TRange&& range) const {
    return [&]<std::size_t... tIdxs>(std::index_sequence<tIdxs...> /*idxd*/) THES_ALWAYS_INLINE {
      return TContainer{get_at<tIdxs>(range)...};
    }(std::make_index_sequence<size<TRange>>{});
  }
};

template<typename TContainer>
inline constexpr ToContainerGenerator<TContainer> to_container{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_TO_CONTAINER_HPP
