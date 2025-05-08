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
#include "thesauros/static-ranges/sinks/for-each.hpp"
#include "thesauros/static-ranges/sinks/to-array.hpp"
#include "thesauros/static-ranges/views/iota.hpp"
#include "thesauros/static-ranges/views/transform.hpp"
#include "thesauros/types/type-tag.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes::star {
template<typename TInner, auto tIdxRange>
struct FilterView {
  TInner inner;

  using IdxRange = std::decay_t<decltype(tIdxRange)>;
  static constexpr std::size_t size = star::size<IdxRange>;

  template<std::size_t tIndex>
  THES_ALWAYS_INLINE constexpr decltype(auto) get(IndexTag<tIndex> /*index*/) const {
    return get_at<get_at<tIndex>(tIdxRange)>(inner);
  }
};

template<auto tIdxRange>
struct OnlyIndicesGenerator : public RangeGeneratorBase {
  template<typename TRange>
  THES_ALWAYS_INLINE constexpr FilterView<TRange, tIdxRange> operator()(TRange&& range) const {
    return {std::forward<TRange>(range)};
  }
};

template<auto tIdxRange>
struct AllExceptIndicesGenerator : public RangeGeneratorBase {
  template<typename TRange>
  THES_ALWAYS_INLINE constexpr auto operator()(TRange&& range) const {
    constexpr std::size_t range_size = star::size<TRange>;

    constexpr auto pair = [&]() THES_ALWAYS_INLINE {
      std::array<std::size_t, range_size> buffer{};
      std::size_t count = 0;

      star::for_each([&](auto i) THES_ALWAYS_INLINE {
        bool contains = false;
        star::for_each([&](auto j) { contains = contains || (i == j); })(tIdxRange);
        if (!contains) {
          buffer[count] = i;
          ++count;
        }
      })(star::iota<0, range_size>);

      return std::make_pair(buffer, count);
    }();
    constexpr auto idxs = star::to_array(star::index_transform<pair.second>(
      [&](auto idx) THES_ALWAYS_INLINE { return std::get<idx>(pair.first); }));

    return FilterView<TRange, idxs>{std::forward<TRange>(range)};
  }
};

template<auto tFun>
struct FilterGenerator : public RangeGeneratorBase {
  template<typename TRange>
  THES_ALWAYS_INLINE constexpr auto operator()(TRange&& range) const {
    auto idx_num = []() THES_ALWAYS_INLINE {
      constexpr std::size_t size = star::size<TRange>;
      std::size_t ctr = 0;
      iota<0, size> | for_each([&](auto idx) THES_ALWAYS_INLINE {
        if (tFun(idx, type_tag<decltype(get_at(std::declval<TRange>(), idx))>)) {
          ++ctr;
        }
      });
      return ctr;
    };
    auto gen_idxs = [&]() THES_ALWAYS_INLINE {
      constexpr std::size_t size = star::size<TRange>;
      std::array<std::size_t, idx_num()> idxs{};
      std::size_t ctr = 0;
      iota<0, size> | for_each([&](auto idx) THES_ALWAYS_INLINE {
        if (tFun(idx, type_tag<decltype(get_at(std::declval<TRange>(), idx))>)) {
          idxs[ctr++] = idx;
        }
      });
      return idxs;
    };
    return FilterView<TRange, gen_idxs()>{std::forward<TRange>(range)};
  }
};

template<std::size_t... tIdxs>
inline constexpr OnlyIndicesGenerator<std::array<std::size_t, sizeof...(tIdxs)>{tIdxs...}>
  only_idxs{};
template<auto tIdxRange>
inline constexpr OnlyIndicesGenerator<tIdxRange> only_range{};

template<std::size_t... tIdxs>
inline constexpr AllExceptIndicesGenerator<std::array<std::size_t, sizeof...(tIdxs)>{tIdxs...}>
  all_except_idxs{};
template<auto tIdxRange>
inline constexpr AllExceptIndicesGenerator<tIdxRange> all_except_range{};

template<std::size_t tBegin, std::size_t tEnd>
inline constexpr OnlyIndicesGenerator<star::iota<tBegin, tEnd>> sub_range{};

template<auto tFun>
inline constexpr FilterGenerator<tFun> filter{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_VIEWS_FILTER_HPP
