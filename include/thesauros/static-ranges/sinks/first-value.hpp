// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_SINKS_FIRST_VALUE_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_SINKS_FIRST_VALUE_HPP

#include <cstddef>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/macropolis/void-macros.hpp"
#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/definitions/type-traits.hpp"

namespace thes::star {
struct FirstValueGenerator : public ConsumerGeneratorBase {
  template<typename Range>
  THES_ALWAYS_INLINE constexpr auto operator()(Range&& range) const {
    constexpr std::size_t size = thes::star::size<Range>;
    using Ret = Value<Range>;
    return work<Ret>(range, std::make_index_sequence<size>{});
  }

  template<typename Ret, std::size_t Head, std::size_t... Tail>
  THES_ALWAYS_INLINE constexpr decltype(auto)
  work(auto& range, std::index_sequence<Head, Tail...> /*idxs*/) const {
    THES_APPLY_VALUED_RETURN(Ret, get_at<Head>(range));
    if constexpr (sizeof...(Tail) > 0) {
      return work<Ret>(range, std::index_sequence<Tail...>{});
    } else {
      THES_RETURN_EMPTY_OPTIONAL(Ret);
    }
  }
};

inline constexpr FirstValueGenerator first_value{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_FIRST_VALUE_HPP
