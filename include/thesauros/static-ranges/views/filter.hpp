// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_FILTER_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_FILTER_HPP

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/definitions/tuple-defs.hpp"
#include "thesauros/static-ranges/sinks/for-each.hpp"
#include "thesauros/static-ranges/sinks/to-array.hpp"
#include "thesauros/static-ranges/views/iota.hpp"
#include "thesauros/static-ranges/views/transform.hpp"
#include "thesauros/types/type-tag.hpp"

namespace thes::star {
template<typename Inner, auto IdxRange>
struct FilterView {
  using IndexRange = std::decay_t<decltype(IdxRange)>;
  static constexpr std::size_t size = star::size<IndexRange>;
  static constexpr TupleDefsMarker tuple_defs_marker{};

  Inner inner;

  template<std::size_t I>
  THES_ALWAYS_INLINE friend constexpr decltype(auto) get(const FilterView& self) {
    return get_at<get_at<I>(IdxRange)>(self.inner);
  }
};

template<auto IdxRange>
struct OnlyIndicesGenerator : public RangeGeneratorBase {
  template<typename TRange>
  THES_ALWAYS_INLINE constexpr FilterView<TRange, IdxRange> operator()(TRange&& range) const {
    return {std::forward<TRange>(range)};
  }
};

template<auto IdxRange>
struct AllExceptIndicesGenerator : public RangeGeneratorBase {
  template<typename TRange>
  THES_ALWAYS_INLINE constexpr auto operator()(TRange&& range) const {
    constexpr std::size_t range_size = star::size<TRange>;

    constexpr auto pair = [&]() THES_ALWAYS_INLINE {
      std::array<std::size_t, range_size> buffer{};
      std::size_t count = 0;

      star::for_each([&](auto i) THES_ALWAYS_INLINE {
        bool contains = false;
        star::for_each([&](auto j) { contains = contains || (i == j); })(IdxRange);
        if (!contains) {
          buffer[count] = i;
          ++count;
        }
      })(star::tagged_iota<0, range_size>);

      return std::make_pair(buffer, count);
    }();
    constexpr auto idxs = star::to_array(star::index_transform<pair.second>(
      [&](auto idx) THES_ALWAYS_INLINE { return std::get<idx>(pair.first); }));

    return FilterView<TRange, idxs>{std::forward<TRange>(range)};
  }
};

template<auto F>
struct FilterGenerator : public RangeGeneratorBase {
  template<typename TRange>
  THES_ALWAYS_INLINE constexpr auto operator()(TRange&& range) const {
    auto idx_num = []() THES_ALWAYS_INLINE {
      constexpr std::size_t size = star::size<TRange>;
      std::size_t ctr = 0;
      tagged_iota<0, size> | for_each([&](auto idx) THES_ALWAYS_INLINE {
        if (F(idx, type_tag<decltype(get_at(std::declval<TRange>(), idx))>)) {
          ++ctr;
        }
      });
      return ctr;
    };
    auto gen_idxs = [&]() THES_ALWAYS_INLINE {
      constexpr std::size_t size = star::size<TRange>;
      std::array<std::size_t, idx_num()> idxs{};
      std::size_t ctr = 0;
      tagged_iota<0, size> | for_each([&](auto idx) THES_ALWAYS_INLINE {
        if (F(idx, type_tag<decltype(get_at(std::declval<TRange>(), idx))>)) {
          idxs[ctr++] = idx;
        }
      });
      return idxs;
    };
    return FilterView<TRange, gen_idxs()>{std::forward<TRange>(range)};
  }
};

template<std::size_t... I>
inline constexpr OnlyIndicesGenerator<std::array<std::size_t, sizeof...(I)>{I...}> only_idxs{};
template<auto IdxRange>
inline constexpr OnlyIndicesGenerator<IdxRange> only_range{};

template<std::size_t... I>
inline constexpr AllExceptIndicesGenerator<std::array<std::size_t, sizeof...(I)>{I...}>
  all_except_idxs{};
template<auto IdxRange>
inline constexpr AllExceptIndicesGenerator<IdxRange> all_except_range{};

template<std::size_t Begin, std::size_t End>
inline constexpr OnlyIndicesGenerator<star::iota<Begin, End>> sub_range{};

template<auto F>
inline constexpr FilterGenerator<F> filter{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_FILTER_HPP
