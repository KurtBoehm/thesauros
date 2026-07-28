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
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/definitions/tuple-defs.hpp"
#include "thesauros/static-ranges/definitions/type-traits.hpp"
#include "thesauros/static-ranges/sinks/for-each.hpp"
#include "thesauros/static-ranges/views/iota.hpp"
#include "thesauros/types/tuple.hpp"

namespace thes::star {
template<typename Ranges>
struct JoinView {
  static constexpr std::size_t size = []<std::size_t... tIdxs>(std::index_sequence<tIdxs...>) {
    return (0UZ + ... + thes::star::size<std::decay_t<Element<tIdxs, Ranges>>>);
  }(std::make_index_sequence<star::size<Ranges>>{});
  static constexpr TupleDefsMarker tuple_defs_marker{};

  Ranges ranges;

  template<std::size_t I>
  requires(I < size)
  THES_ALWAYS_INLINE friend constexpr auto get(const JoinView& self) {
    constexpr auto pair = []() THES_ALWAYS_INLINE {
      std::size_t sum = 0;
      std::optional<std::pair<std::size_t, std::size_t>> out{};
      star::for_each([&](auto idx) THES_ALWAYS_INLINE {
        constexpr std::size_t idx_size = star::size<std::decay_t<Element<idx, Ranges>>>;
        if (sum <= I && I < sum + idx_size) {
          if (out.has_value()) {
            std::abort();
          }
          out = std::make_pair(idx.value, I - sum);
        }
        sum += idx_size;
      })(star::iota<0, star::size<Ranges>>);
      return *out;
    }();
    return get_at<pair.second>(get_at<pair.first>(self.ranges));
  }
};
template<typename... Ranges>
JoinView(Ranges&&...) -> JoinView<Ranges...>;

template<typename... Ranges>
requires(sizeof...(Ranges) > 0)
THES_ALWAYS_INLINE inline constexpr auto joined(Ranges&&... ranges) {
  return JoinView{thes::make_tuple(std::forward<Ranges>(ranges)...)};
}

template<typename NestedRange>
THES_ALWAYS_INLINE inline constexpr auto flattened(NestedRange&& ranges) {
  return JoinView{std::forward<NestedRange>(ranges)};
}

struct JoinGenerator : public RangeGeneratorBase {
  template<typename Ranges>
  THES_ALWAYS_INLINE constexpr JoinView<Ranges> operator()(Ranges&& range) const {
    return {std::forward<Ranges>(range)};
  }
};
inline constexpr JoinGenerator join;
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_JOIN_HPP
