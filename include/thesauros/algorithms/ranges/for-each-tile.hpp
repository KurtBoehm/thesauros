#ifndef INCLUDE_THESAUROS_ALGORITHMS_RANGES_FOR_EACH_TILE_HPP
#define INCLUDE_THESAUROS_ALGORITHMS_RANGES_FOR_EACH_TILE_HPP

#include <array>
#include <cassert>
#include <cstddef>

#include "thesauros/math/arithmetic.hpp"
#include "thesauros/utility/inlining.hpp"
#include "thesauros/utility/no-op.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/static-ranges/definitions/type-traits.hpp"
#include "thesauros/utility/value-tag.hpp"

namespace thes {
enum struct IterDirection : bool { FORWARD, BACKWARD };

template<typename TSize, std::size_t tDimNum>
struct IndexPosition {
  static constexpr std::size_t dimension_num = tDimNum;

  explicit operator TSize() const {
    return index;
  }

  std::array<TSize, dimension_num> position;
  TSize index;
};

// Create tiles
template<IterDirection tDirection, typename TRanges, typename TFixedAxes>
inline constexpr void for_each_tile(const TRanges& ranges, const auto& tile_sizes,
                                    const TFixedAxes& fixed_axes, auto&& fun) {
  using Size = star::Value<star::Value<TRanges>>;
  constexpr std::size_t dim_num = star::size<TRanges>;

  if constexpr (dim_num == 0) {
    return;
  } else {
    auto impl = [&](auto dim, auto rec, auto... args) THES_ALWAYS_INLINE {
      static_assert(dim <= dim_num && sizeof...(args) == dim);
      if constexpr (dim == dim_num) {
        fun(args...);
      } else if constexpr (TFixedAxes::contains(dim)) {
        rec(index_tag<dim + 1>, rec, args..., fixed_axes.get(dim));
      } else {
        const auto [begin, end] = star::get_at<dim>(ranges);
        const Size tile_size = star::get_at<dim>(tile_sizes);
        if constexpr (tDirection == IterDirection::FORWARD) {
          for (Size i = begin; i < end; i += tile_size) {
            rec(index_tag<dim + 1>, rec, args..., std::make_pair(i, std::min(i + tile_size, end)));
          }
        } else {
          const Size size = end - begin;
          const Size size_tile = div_ceil(size, tile_size) * tile_size;
          for (Size i = begin + size_tile; i > 0; i -= tile_size) {
            rec(index_tag<dim + 1>, rec, args..., std::make_pair(i - tile_size, std::min(i, end)));
          }
        }
      }
    };
    impl(index_tag<0>, impl);
  }
}
template<IterDirection tDirection, typename TRanges, typename TFixedAxes>
inline constexpr void for_each_tile(const TRanges& ranges, const auto& tile_sizes,
                                    const TFixedAxes& fixed_axes, auto&& full_fun, auto&& part_fun,
                                    AnyIndexTag auto vec_size) {
  using Size = star::Value<star::Value<TRanges>>;
  constexpr std::size_t dim_num = star::size<TRanges>;

  if constexpr (dim_num == 0) {
    return;
  } else {
    auto impl = [&](auto dim, auto rec, auto... args) THES_ALWAYS_INLINE {
      static_assert(dim < dim_num && sizeof...(args) == dim);
      const auto [begin, end] = star::get_at<dim>(ranges);
      const Size tile_size = star::get_at<dim>(tile_sizes);

      if constexpr (dim + 1 == dim_num) {
        static_assert(!TFixedAxes::contains(dim));
        assert(tile_size % vec_size == 0);

        if constexpr (tDirection == IterDirection::FORWARD) {
          Size i = begin;
          for (; i + tile_size <= end; i += tile_size) {
            full_fun(args..., std::make_pair(i, i + tile_size));
          }
          if (i != end) {
            part_fun(args..., std::make_pair(i, end));
          }
        } else {
          const Size full_tile_end = end - ((end - begin) % tile_size);
          if (full_tile_end != end) {
            part_fun(args..., std::make_pair(full_tile_end, end));
          }
          for (Size i = full_tile_end; i > 0; i -= tile_size) {
            full_fun(args..., std::make_pair(i - tile_size, i));
          }
        }
      } else if constexpr (TFixedAxes::contains(dim)) {
        const auto idx = fixed_axes.get(dim);
        rec(index_tag<dim + 1>, rec, args..., std::make_pair(idx, idx + 1));
      } else {
        if constexpr (tDirection == IterDirection::FORWARD) {
          for (Size i = begin; i < end; i += tile_size) {
            rec(index_tag<dim + 1>, rec, args..., std::make_pair(i, std::min(i + tile_size, end)));
          }
        } else {
          const Size size = end - begin;
          const Size size_tile = div_ceil(size, tile_size) * tile_size;
          for (Size i = begin + size_tile; i > begin; i -= tile_size) {
            rec(index_tag<dim + 1>, rec, args..., std::make_pair(i - tile_size, std::min(i, end)));
          }
        }
      }
    };
    impl(index_tag<0>, impl);
  }
}

// Iterate over a tile
template<IterDirection tDirection, typename TRanges>
inline constexpr void tile_for_each(const auto& multi_size, const TRanges& ranges, auto&& fun) {
  using Range = star::Value<TRanges>;
  using Size = star::Value<Range>;
  constexpr std::size_t dim_num = star::size<TRanges>;
  using IndexPos = IndexPosition<Size, dim_num>;

  auto impl = [&](auto dim, auto rec, auto index, auto... args) THES_ALWAYS_INLINE {
    const auto [begin, end] = star::get_at<dim>(ranges);

    if constexpr (tDirection == IterDirection::FORWARD) {
      for (Size i = begin; i < end; ++i) {
        if constexpr (dim + 1 < dim_num) {
          const auto factor = multi_size.after_size(dim);
          rec(index_tag<dim + 1>, rec, index + i * factor, args..., i);
        } else {
          fun(IndexPos{{args..., i}, index + i});
        }
      }
    } else {
      for (Size i = end; i > begin; --i) {
        if constexpr (dim + 1 < dim_num) {
          const auto factor = multi_size.after_size(dim);
          rec(index_tag<dim + 1>, rec, index + (i - 1) * factor, args..., i - 1);
        } else {
          fun(IndexPos{{args..., i - 1}, index + (i - 1)});
        }
      }
    }
  };
  impl(index_tag<0>, impl, Size{0});
}

template<IterDirection tDirection, typename TRanges>
inline constexpr void tile_for_each(const auto& multi_size, const TRanges& ranges, auto&& full_fun,
                                    auto&& part_fun, AnyIndexTag auto vec_size,
                                    AnyBoolTag auto has_part) {
  using Range = star::Value<TRanges>;
  using Size = star::Value<Range>;
  constexpr std::size_t dim_num = star::size<TRanges>;
  using IndexPos = IndexPosition<Size, dim_num>;

  auto impl = [&](auto dim, auto rec, auto index, auto... args) THES_ALWAYS_INLINE {
    const auto [begin, end] = star::get_at<dim>(ranges);

    if constexpr (tDirection == IterDirection::FORWARD) {
      if constexpr (dim + 1 < dim_num) {
        for (Size i = begin; i < end; ++i) {
          const auto factor = multi_size.after_size(dim);
          rec(index_tag<dim + 1>, rec, index + i * factor, args..., i);
        }
      } else {
        if constexpr (has_part) {
          Size i = begin;
          for (; i + vec_size < end; i += vec_size) {
            full_fun(IndexPos{{args..., i}, index + i});
          }
          if (i != end) {
            part_fun(IndexPos{{args..., i}, index + i}, end - i);
          }
        } else {
          assert((end - begin) % vec_size == 0);
          for (Size i = begin; i < end; i += vec_size) {
            full_fun(IndexPos{{args..., i}, index + i});
          }
        }
      }
    } else {
      if constexpr (dim + 1 < dim_num) {
        for (Size i = end; i > begin; --i) {
          const auto factor = multi_size.after_size(dim);
          const Size idx = i - 1;
          rec(index_tag<dim + 1>, rec, index + idx * factor, args..., idx);
        }
      } else {
        if constexpr (has_part) {
          const Size full_vec_end = end - ((end - begin) % vec_size);
          if (full_vec_end != end) {
            part_fun(IndexPos{{args..., full_vec_end}, index + full_vec_end}, end - full_vec_end);
          }
          for (Size i = full_vec_end; i > begin; i -= vec_size) {
            const Size idx = i - vec_size;
            full_fun(IndexPos{{args..., idx}, index + idx});
          }
        } else {
          assert((end - begin) % vec_size == 0);
          for (Size i = end; i > begin; i -= vec_size) {
            const Size idx = i - vec_size;
            full_fun(IndexPos{{args..., idx}, index + idx});
          }
        }
      }
    }
  };
  impl(index_tag<0>, impl, Size{0});
}

template<IterDirection tDirection, typename TRanges, typename TFixedAxes>
inline constexpr void tiled_for_each(const auto& multi_size, const TRanges& ranges,
                                     const auto& tile_sizes, const TFixedAxes& fixed_axes,
                                     auto&& fun) {
  thes::for_each_tile<tDirection>(ranges, tile_sizes, fixed_axes, [&](auto... args) {
    thes::tile_for_each<tDirection>(multi_size, std::array{args...}, fun);
  });
}
template<IterDirection tDirection, typename TRanges, typename TFixedAxes>
inline constexpr void tiled_for_each(const auto& multi_size, const TRanges& ranges,
                                     const auto& tile_sizes, const TFixedAxes& fixed_axes,
                                     auto&& full_fun, auto&& part_fun, AnyIndexTag auto vec_size) {
  thes::for_each_tile<tDirection>(
    ranges, tile_sizes, fixed_axes,
    [&](auto... args) {
      thes::tile_for_each<tDirection>(multi_size, std::array{args...}, full_fun, thes::NoOp{},
                                      vec_size, thes::false_tag);
    },
    [&](auto... args) {
      thes::tile_for_each<tDirection>(multi_size, std::array{args...}, full_fun, part_fun, vec_size,
                                      thes::true_tag);
    },
    vec_size);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_ALGORITHMS_RANGES_FOR_EACH_TILE_HPP
