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
template<typename Container>
struct ToContainerGenerator : public ConsumerGeneratorBase {
  template<typename R>
  THES_ALWAYS_INLINE constexpr auto operator()(R&& range) const {
    return [&]<std::size_t... I>(std::index_sequence<I...> /*idxd*/) THES_ALWAYS_INLINE {
      return Container{get_at<I>(range)...};
    }(std::make_index_sequence<size<R>>{});
  }
};

template<typename Container>
inline constexpr ToContainerGenerator<Container> to_container{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_TO_CONTAINER_HPP
