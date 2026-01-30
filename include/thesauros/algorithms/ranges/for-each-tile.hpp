// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_ALGORITHMS_RANGES_FOR_EACH_TILE_HPP
#define INCLUDE_THESAUROS_ALGORITHMS_RANGES_FOR_EACH_TILE_HPP

#include <array>
#include <cassert>
#include <cstddef>
#include <type_traits>

#include "thesauros/functional/no-op.hpp"
#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/ranges.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/definitions/type-traits.hpp"
#include "thesauros/types/type-sequence.hpp"
#include "thesauros/types/type-tag.hpp"
#include "thesauros/types/value-tag.hpp"
#include "thesauros/utility/tuple.hpp"

namespace thes {
/** Iteration direction (forward or backward). */
enum struct IterDirection : bool { FORWARD, BACKWARD };

/**
 * Pair of flat index and multi-dimensional position.
 *
 * @tparam TIdx flat index type
 * @tparam TPos multi-dimensional position type (tuple/array-like)
 */
template<typename TIdx, typename TPos>
struct IndexPosition {
  /** Flat index type. */
  using Index = TIdx;
  /** Multi-dimensional position type. */
  using Position = TPos;
  /** Number of dimensions in `Position`. */
  static constexpr std::size_t dimension_num = star::size<Position>;

  /** Implicit conversion to flat index. */
  explicit constexpr operator Index() const {
    return index;
  }

  /**
   * Add an offset to the flat index.
   *
   * @param rhs offset added to `index`
   * @return `index + rhs`
   */
  friend constexpr decltype(auto) operator+(IndexPosition lhs, auto rhs)
  requires requires(Index lhs_index) { lhs_index + rhs; }
  {
    return lhs.index + rhs;
  }

  /** Flat index. */
  Index index{};
  /** Multi-dimensional coordinates. */
  Position position{};
};

/** `true` for any `IndexPosition<Idx, Pos>` specialization. */
template<typename>
struct AnyIndexPositionTrait : public std::false_type {};

template<typename TSize, typename TPos>
struct AnyIndexPositionTrait<IndexPosition<TSize, TPos>> : public std::true_type {};

/** Concept matching any `IndexPosition` specialization. */
template<typename T>
concept AnyIndexPosition = AnyIndexPositionTrait<T>::value;

/**
 * Common nested value type of a tuple-like of ranges.
 *
 * @tparam TRanges tuple-like type whose elements are ranges
 */
template<typename TRanges>
using NestedValueType = TransformedTypeSeq<star::ValueSeq<TRanges>, star::Value>::Unique;

namespace detail {
/**
 * Iterate over a 1D interval in tiles.
 *
 * For the half-open interval `[begin, end)`, repeatedly calls:
 *
 * - `full_fun(range_size(i, tile_size))` for each full tile
 * - `part_fun(range(i, end))` for the trailing partial tile (if any)
 *
 * Direction is controlled by `tDir`.
 *
 * @tparam tDir  iteration direction
 * @tparam TSize index type
 * @param begin     start index
 * @param end       end index
 * @param tile_size tile size
 * @param full_fun  callback for full tiles
 * @param part_fun  callback for the partial tail tile
 */
template<IterDirection tDir, typename TSize>
THES_ALWAYS_INLINE inline constexpr void
for_each_tile_unroll(TSize begin, TSize end, auto tile_size, auto&& full_fun, auto&& part_fun) {
  if constexpr (tDir == IterDirection::FORWARD) {
    TSize i = begin;
    for (; i + tile_size <= end; i += tile_size) {
      full_fun(range_size(i, tile_size));
    }
    if (i != end) {
      part_fun(range(i, end));
    }
  } else {
    const TSize full_tile_end = end - ((end - begin) % tile_size);
    if (full_tile_end != end) {
      part_fun(range(full_tile_end, end));
    }
    for (TSize i = full_tile_end; i > begin; i -= tile_size) {
      full_fun(range_size(i - tile_size, tile_size));
    }
  }
}

/**
 * Multi-dimensional tiling of a hyperrectangle.
 *
 * Recursively tiles each dimension of `ranges` with the given per-dimension `tile_sizes`.
 * For each resulting tile, calls:
 *
 * - `full_fun(...)` when all dimensions form full tiles
 * - `part_fun(...)` when any dimension has a partial tile
 *
 * Fixed axes are not tiled; instead their index is taken from `fixed_axes`.
 *
 * @tparam tDir       iteration direction
 * @tparam TRanges    tuple-like of ranges defining the hyperrectangle
 * @tparam TFixedAxes container describing fixed axes
 * @param ranges     per-dimension ranges (hyperrectangle)
 * @param tile_sizes per-dimension tile sizes
 * @param fixed_axes indices and values for fixed axes
 * @param full_fun   callback for full tiles
 * @param part_fun   callback for partial tiles
 */
template<IterDirection tDir, typename TRanges, typename TFixedAxes>
THES_ALWAYS_INLINE inline constexpr void
for_each_tile(const TRanges& ranges, const auto& tile_sizes, const TFixedAxes& fixed_axes,
              auto&& full_fun, auto&& part_fun) {
  using Size = NestedValueType<TRanges>;
  constexpr std::size_t dim_num = star::size<TRanges>;

  if constexpr (dim_num == 0) {
    return;
  } else {
    auto impl = [&](auto dim, auto rec, auto... args) THES_ALWAYS_INLINE {
      static_assert(dim < dim_num && sizeof...(args) == dim);

      const auto dim_range = star::get_at<dim>(ranges);
      const auto begin = dim_range.begin_value();
      const auto end = dim_range.end_value();
      const auto tile_size = star::get_at<dim>(tile_sizes);

      static_assert(!TFixedAxes::contains(thes::index_tag<dim_num - 1>));

      if constexpr (dim + 1 == dim_num) {
        for_each_tile_unroll<tDir>(
          begin, end, tile_size, [&](auto r) { full_fun(args..., r); },
          [&](auto r) { part_fun(args..., r); });
      } else if constexpr (TFixedAxes::contains(dim)) {
        const auto idx = fixed_axes.get(dim);
        rec(index_tag<dim + 1>, rec, args..., range_size(idx, value_tag<Size, 1>));
      } else {
        auto op = [&](auto r) { rec(index_tag<dim + 1>, rec, args..., r); };
        for_each_tile_unroll<tDir>(begin, end, tile_size, op, op);
      }
    };

    impl(index_tag<0>, impl);
  }
}
} // namespace detail

/**
 * Split a hyperrectangle into tiles and iterate over all tiles.
 *
 * This is the simple version where full and partial tiles are treated
 * identically and passed to the same callback.
 *
 * For each tile, `fun` is called with one subrange per dimension.
 *
 * @tparam tDir       iteration direction
 * @tparam TRanges    tuple-like of ranges
 * @tparam TFixedAxes container describing fixed axes
 * @param ranges      overall per-dimension ranges
 * @param tile_sizes  per-dimension tile sizes
 * @param fixed_axes  indices and values for fixed axes
 * @param fun         tile callback: `fun(range_dim0, range_dim1, ...)`
 */
template<IterDirection tDir, typename TRanges, typename TFixedAxes>
THES_ALWAYS_INLINE inline constexpr void for_each_tile(const TRanges& ranges,
                                                       const auto& tile_sizes,
                                                       const TFixedAxes& fixed_axes, auto&& fun) {
  detail::for_each_tile<tDir>(ranges, tile_sizes, fixed_axes, fun, fun);
}

/**
 * Tiled iteration with a vector-size constraint.
 *
 * Requires that, for all non-fixed dimensions, the corresponding tile size is a multiple
 * of `vec_size`. Full/partial tiles are delegated to `full_fun` and `part_fun`, respectively.
 *
 * @tparam tDir       iteration direction
 * @tparam TRanges    tuple-like of ranges
 * @tparam TFixedAxes container describing fixed axes
 * @param ranges      overall per-dimension ranges
 * @param tile_sizes  per-dimension tile sizes
 * @param fixed_axes  indices and values for fixed axes
 * @param full_fun    callback for full tiles
 * @param part_fun    callback for partial tiles
 * @param vec_size    vector width (index tag)
 */
template<IterDirection tDir, typename TRanges, typename TFixedAxes>
THES_ALWAYS_INLINE inline constexpr void
for_each_tile(const TRanges& ranges, const auto& tile_sizes, const TFixedAxes& fixed_axes,
              auto&& full_fun, auto&& part_fun, [[maybe_unused]] AnyIndexTag auto vec_size) {
  assert(star::static_apply<star::size<TRanges>>([&]<std::size_t... tIdxs>() {
    return (... && (TFixedAxes::contains(index_tag<tIdxs>) ||
                    star::get_at<tIdxs>(tile_sizes) % vec_size == 0));
  }));

  detail::for_each_tile<tDir>(ranges, tile_sizes, fixed_axes, full_fun, part_fun);
}

/**
 * Iterate over all cells within a single tile.
 *
 * Visits every element in `ranges` in linear index order, also providing
 * its multi-dimensional coordinates.
 *
 * @tparam tDir    iteration direction
 * @tparam TRanges tuple-like of per-dimension ranges
 * @tparam TIdx    flat index type
 * @param multi_size object providing `after_size(dim)` for layout
 * @param ranges     tile ranges
 * @param fun        callback: `fun(IndexPosition<Index, Position>)`
 * @param tag        flat index type tag
 */
template<IterDirection tDir, typename TRanges, typename TIdx = NestedValueType<TRanges>>
THES_ALWAYS_INLINE inline constexpr void tile_for_each(const auto& multi_size,
                                                       const TRanges& ranges, auto&& fun,
                                                       TypeTag<TIdx> /*tag*/ = {}) {
  using Size = NestedValueType<TRanges>;
  constexpr std::size_t dim_num = star::size<TRanges>;
  using IndexPos = IndexPosition<TIdx, std::array<Size, dim_num>>;

  auto impl = [&](auto dim, auto&& rec, auto index, auto... coords) THES_ALWAYS_INLINE {
    const auto range = star::get_at<dim>(ranges);
    const auto begin = range.begin_value();
    const auto end = range.end_value();

    if constexpr (tDir == IterDirection::FORWARD) {
      for (Size i = begin; i < end; ++i) {
        if constexpr (dim + 1 < dim_num) {
          const auto factor = multi_size.after_size(dim);
          rec(index_tag<dim + 1>, rec, index + i * factor, coords..., i);
        } else {
          fun(IndexPos{TIdx{index + i}, {coords..., i}});
        }
      }
    } else {
      for (Size i = end; i > begin; --i) {
        if constexpr (dim + 1 < dim_num) {
          const auto factor = multi_size.after_size(dim);
          rec(index_tag<dim + 1>, rec, index + (i - 1) * factor, coords..., i - 1);
        } else {
          fun(IndexPos{TIdx{index + (i - 1)}, {coords..., i - 1}});
        }
      }
    }
  };

  impl(index_tag<0>, impl, Size{0});
}

/**
 * Iterate over all cells in a tile, vectorizing the inner dimension.
 *
 * The last dimension is split into chunks of `vec_size`. For each full vector,
 * `full_fun` is called; for the (optional) remaining tail, `part_fun` is called.
 *
 * @tparam tDir    iteration direction
 * @tparam TRanges tuple-like of per-dimension ranges
 * @tparam TIdx    flat index type
 * @param multi_size object providing `after_size(dim)` for layout
 * @param ranges     tile ranges
 * @param full_fun   callback for full vectors
 * @param part_fun   callback for partial tails
 * @param vec_size   vector width tag
 * @param has_part   bool tag indicating whether a tail may exist
 * @param tag        flat index type tag
 */
template<IterDirection tDir, typename TRanges, typename TIdx = NestedValueType<TRanges>>
THES_ALWAYS_INLINE inline constexpr void
tile_for_each(const auto& multi_size, const TRanges& ranges, auto&& full_fun, auto&& part_fun,
              AnyIndexTag auto vec_size, AnyBoolTag auto has_part, TypeTag<TIdx> /*tag*/ = {}) {
  using Size = NestedValueType<TRanges>;
  constexpr std::size_t dim_num = star::size<TRanges>;
  using IndexPos = IndexPosition<TIdx, std::array<Size, dim_num>>;
  constexpr Size vsize = static_cast<Size>(vec_size);

  auto impl = [&](auto dim, auto&& rec, auto index, auto... coords) THES_ALWAYS_INLINE {
    const auto range = star::get_at<dim>(ranges);

    if constexpr (tDir == IterDirection::FORWARD) {
      if constexpr (dim + 1 < dim_num) {
        for (const Size i : range) {
          const auto factor = multi_size.after_size(dim);
          rec(index_tag<dim + 1>, rec, index + i * factor, coords..., i);
        }
      } else {
        const auto begin = range.begin_value();
        const auto end = range.end_value();

        if constexpr (has_part) {
          Size i = begin;
          for (; i + vsize <= end; i += vsize) {
            full_fun(IndexPos{TIdx{index + i}, {coords..., i}});
          }
          if (i != end) {
            part_fun(IndexPos{TIdx{index + i}, {coords..., i}}, end - i);
          }
        } else {
          assert(range.size() % vsize == 0);
          for (Size i = begin; i < end; i += vsize) {
            full_fun(IndexPos{TIdx{index + i}, {coords..., i}});
          }
        }
      }
    } else {
      if constexpr (dim + 1 < dim_num) {
        for (const Size i : reversed(range)) {
          const auto factor = multi_size.after_size(dim);
          rec(index_tag<dim + 1>, rec, index + i * factor, coords..., i);
        }
      } else {
        const auto begin = range.begin_value();
        const auto end = range.end_value();

        if constexpr (has_part) {
          const Size full_vec_end = end - (range.size() % vsize);
          if (full_vec_end != end) {
            part_fun(IndexPos{TIdx{index + full_vec_end}, {coords..., full_vec_end}},
                     end - full_vec_end);
          }
          for (Size i = full_vec_end; i > begin; i -= vsize) {
            const Size idx = i - vsize;
            full_fun(IndexPos{TIdx{index + idx}, {coords..., idx}});
          }
        } else {
          assert(range.size() % vsize == 0);
          for (Size i = end; i > begin; i -= vsize) {
            const Size idx = i - vsize;
            full_fun(IndexPos{TIdx{index + idx}, {coords..., idx}});
          }
        }
      }
    }
  };

  impl(index_tag<0>, impl, Size{0});
}

/**
 * Tiled iteration over all elements of a hyperrectangle.
 *
 * Combines `for_each_tile` with scalar `tile_for_each` to visit each element in tile-major order.
 *
 * @tparam tDir    iteration direction
 * @tparam TRanges tuple-like of per-dimension ranges
 * @tparam TIdx    flat index type
 * @param multi_size object providing `after_size(dim)` for layout
 * @param ranges     overall per-dimension ranges
 * @param tile_sizes per-dimension tile sizes
 * @param fixed_axes indices and values for fixed axes
 * @param fun        element callback: `fun(IndexPosition)`
 * @param tag        flat index type tag
 */
template<IterDirection tDir, typename TRanges, typename TIdx = NestedValueType<TRanges>>
THES_ALWAYS_INLINE inline constexpr void
tiled_for_each(const auto& multi_size, const TRanges& ranges, const auto& tile_sizes,
               const auto& fixed_axes, auto&& fun, TypeTag<TIdx> tag = {}) {
  for_each_tile<tDir>(ranges, tile_sizes, fixed_axes, [&](auto... tile_ranges) THES_ALWAYS_INLINE {
    tile_for_each<tDir>(multi_size, Tuple{std::move(tile_ranges)...}, fun, tag);
  });
}

/**
 * Vectorized tiled iteration over all elements of a hyperrectangle.
 *
 * Performs tiled iteration with an inner vector width `vec_size`.
 * Full and partial tiles and vectors are distinguished:
 *
 * - `full_fun` is called for full vectors.
 * - `part_fun` is called for the remaining partial vector tails.
 *
 * @tparam tDir    iteration direction
 * @tparam TRanges tuple-like of per-dimension ranges
 * @tparam TIdx    flat index type
 * @param multi_size object providing `after_size(dim)` for layout
 * @param ranges     overall per-dimension ranges
 * @param tile_sizes per-dimension tile sizes
 * @param fixed_axes indices and values for fixed axes
 * @param full_fun   callback for full vectors
 * @param part_fun   callback for partial vectors
 * @param vec_size   vector width tag
 * @param tag        flat index type tag
 */
template<IterDirection tDir, typename TRanges, typename TIdx = NestedValueType<TRanges>>
THES_ALWAYS_INLINE inline constexpr void
tiled_for_each(const auto& multi_size, const TRanges& ranges, const auto& tile_sizes,
               const auto& fixed_axes, auto&& full_fun, auto&& part_fun, AnyIndexTag auto vec_size,
               TypeTag<TIdx> tag = {}) {
  for_each_tile<tDir>(
    ranges, tile_sizes, fixed_axes,
    // full tiles
    [&](auto... tile_ranges) THES_ALWAYS_INLINE {
      tile_for_each<tDir>(multi_size, Tuple{std::move(tile_ranges)...}, full_fun, NoOp{}, vec_size,
                          /*has_part=*/false_tag, tag);
    },
    // partial tiles
    [&](auto... tile_ranges) THES_ALWAYS_INLINE {
      tile_for_each<tDir>(multi_size, Tuple{std::move(tile_ranges)...}, full_fun, part_fun,
                          /*has_part=*/vec_size, true_tag, tag);
    },
    vec_size);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_ALGORITHMS_RANGES_FOR_EACH_TILE_HPP
