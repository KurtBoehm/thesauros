// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_JOIN_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_JOIN_HPP

#include <cstddef>
#include <cstdlib>
#include <optional>
#include <type_traits>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/printable.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/definitions/type-traits.hpp"
#include "thesauros/static-ranges/sinks/for-each.hpp"
#include "thesauros/static-ranges/views/iota.hpp"
#include "thesauros/types/value-tag.hpp"
#include "thesauros/utility/tuple.hpp"

namespace thes::star {
template<typename TRanges>
struct JoinView {
  static constexpr std::size_t size = []<std::size_t... tIdxs>(std::index_sequence<tIdxs...>) {
    return (... + thes::star::size<std::decay_t<Element<tIdxs, TRanges>>>);
  }(std::make_index_sequence<star::size<TRanges>>{});
  static constexpr PrintableMarker printable{};

  TRanges ranges;

  template<std::size_t tIndex>
  requires(tIndex < size)
  THES_ALWAYS_INLINE constexpr auto get(IndexTag<tIndex> /*index*/) const {
    constexpr auto pair = []() THES_ALWAYS_INLINE {
      std::size_t sum = 0;
      std::optional<std::pair<std::size_t, std::size_t>> out{};
      star::for_each([&](auto idx) THES_ALWAYS_INLINE {
        constexpr std::size_t idx_size = star::size<std::decay_t<Element<idx, TRanges>>>;
        if (sum <= tIndex && tIndex < sum + idx_size) {
          if (out.has_value()) {
            std::abort();
          }
          out = std::make_pair(idx.value, tIndex - sum);
        }
        sum += idx_size;
      })(star::iota<0, star::size<TRanges>>);
      return *out;
    }();
    return get_at<pair.second>(get_at<pair.first>(ranges));
  }
};
template<typename... TRanges>
JoinView(TRanges&&...) -> JoinView<TRanges...>;

template<typename... TRanges>
requires(sizeof...(TRanges) > 0)
THES_ALWAYS_INLINE inline constexpr auto joined(TRanges&&... ranges) {
  return JoinView{Tuple{std::forward<TRanges>(ranges)...}};
}

template<typename TRangeRange>
THES_ALWAYS_INLINE inline constexpr auto flattened(TRangeRange&& ranges) {
  return JoinView{std::forward<TRangeRange>(ranges)};
}

struct JoinGenerator : public RangeGeneratorBase {
  template<typename TRanges>
  THES_ALWAYS_INLINE constexpr JoinView<TRanges> operator()(TRanges&& range) const {
    return {std::forward<TRanges>(range)};
  }
};
inline constexpr JoinGenerator join;
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_JOIN_HPP
