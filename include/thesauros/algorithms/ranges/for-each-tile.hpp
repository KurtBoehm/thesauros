#ifndef INCLUDE_THESAUROS_ALGORITHMS_RANGES_FOR_EACH_TILE_HPP
#define INCLUDE_THESAUROS_ALGORITHMS_RANGES_FOR_EACH_TILE_HPP

#include <array>
#include <cassert>
#include <cstddef>

#include "thesauros/math/arithmetic.hpp"
#include "thesauros/utility/inlining.hpp"
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
template<IterDirection tDirection, typename TSizes, typename TFixedAxes>
inline constexpr void for_each_tile(const TSizes& sizes, const auto& tile_sizes,
                                    const TFixedAxes& fixed_axes, auto&& fun) {
  using Size = star::Value<TSizes>;
  constexpr std::size_t dim_num = star::size<TSizes>;

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
        const Size size = star::get_at<dim>(sizes);
        const Size tile_size = star::get_at<dim>(tile_sizes);
        if constexpr (tDirection == IterDirection::FORWARD) {
          for (Size i = 0; i < size; i += tile_size) {
            rec(index_tag<dim + 1>, rec, args..., std::make_pair(i, std::min(i + tile_size, size)));
          }
        } else {
          const Size end_tile = div_ceil(size, tile_size) * tile_size;
          for (Size i = end_tile; i > 0; i -= tile_size) {
            rec(index_tag<dim + 1>, rec, args..., std::make_pair(i - tile_size, std::min(i, size)));
          }
        }
      }
    };
    impl(index_tag<0>, impl);
  }
}

// Create tiles
template<IterDirection tDirection, typename TSizes>
inline constexpr void for_each_tile_vectorized(const TSizes& sizes, const auto& tile_sizes,
                                               auto&& full_fun, auto&& part_fun,
                                               AnyIndexTag auto vec_size) {
  using Size = star::Value<TSizes>;
  constexpr std::size_t dim_num = star::size<TSizes>;

  if constexpr (dim_num == 0) {
    return;
  } else {
    auto impl = [&](auto dim, auto rec, auto... args) THES_ALWAYS_INLINE {
      static_assert(dim < dim_num && sizeof...(args) == dim);
      const Size size = star::get_at<dim>(sizes);
      const Size tile_size = star::get_at<dim>(tile_sizes);

      if constexpr (dim + 1 == dim_num) {
        assert(tile_size % vec_size == 0);

        if constexpr (tDirection == IterDirection::FORWARD) {
          Size i = 0;
          for (; i + tile_size <= size; i += tile_size) {
            full_fun(args..., std::make_pair(i, i + tile_size));
          }
          if (i != size) {
            part_fun(args..., std::make_pair(i, size));
          }
        } else {
          const Size full_tile_end = size - (size % tile_size);
          if (full_tile_end != size) {
            part_fun(args..., std::make_pair(full_tile_end, size));
          }
          for (Size i = full_tile_end; i > 0; i -= tile_size) {
            full_fun(args..., std::make_pair(i - tile_size, i));
          }
        }
      } else {
        if constexpr (tDirection == IterDirection::FORWARD) {
          for (Size i = 0; i < size; i += tile_size) {
            rec(index_tag<dim + 1>, rec, args..., std::make_pair(i, std::min(i + tile_size, size)));
          }
        } else {
          const Size end_tile = div_ceil(size, tile_size) * tile_size;
          for (Size i = end_tile; i > 0; i -= tile_size) {
            rec(index_tag<dim + 1>, rec, args..., std::make_pair(i - tile_size, std::min(i, size)));
          }
        }
      }
    };
    impl(index_tag<0>, impl);
  }
}

template<IterDirection tDirection, typename TRanges>
inline constexpr void iterate_tile(const auto& multi_size, const TRanges& ranges, auto&& fun) {
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
inline constexpr void iterate_tile(const auto& multi_size, const TRanges& ranges, auto&& full_fun,
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
} // namespace thes

#endif // INCLUDE_THESAUROS_ALGORITHMS_RANGES_FOR_EACH_TILE_HPP
