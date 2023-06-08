#ifndef INCLUDE_THESAUROS_ALGORITHMS_RANGES_FOR_EACH_TILE_HPP
#define INCLUDE_THESAUROS_ALGORITHMS_RANGES_FOR_EACH_TILE_HPP

#include <array>
#include <cstddef>

#include "thesauros/math/arithmetic.hpp"
#include "thesauros/utility/inlining.hpp"
#include "thesauros/utility/static-ranges/definitions/element-type.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/value-tag.hpp"

namespace thes {
enum struct IterDirection { FORWARD, BACKWARD };

template<typename TSize, std::size_t tDimNum>
struct IndexPosition {
  static constexpr std::size_t dimension_num = tDimNum;

  operator TSize() const {
    return index;
  }

  std::array<TSize, dimension_num> position;
  TSize index;
};

// Create tiles
template<IterDirection tDirection, typename TSizes, typename TTileSizes, typename TFixedAxes,
         typename TFun>
inline constexpr void for_each_tile(const TSizes& sizes, const TTileSizes& tile_sizes,
                                    const TFixedAxes& fixed_axes, TFun&& fun) {
  using Size = star::ElementType<TSizes>;
  constexpr std::size_t dim_num = star::size<TSizes>;

  if constexpr (dim_num == 0) {
    return;
  } else {
    auto impl = [&](auto dim, auto rec, auto... args) THES_ALWAYS_INLINE {
      static_assert(dim <= dim_num);
      static_assert(sizeof...(args) == dim);

      if constexpr (dim == dim_num) {
        fun(args...);
      } else if constexpr (TFixedAxes::template contains<dim>) {
        rec(index_tag<dim + 1>, rec, args..., fixed_axes.template get<dim>());
      } else {
        const Size size = star::get_at<dim>(sizes);
        const Size tile_size = star::get_at<dim>(tile_sizes);
        if constexpr (tDirection == IterDirection::FORWARD) {
          for (Size i = 0; i < size; i += tile_size) {
            rec(index_tag<dim + 1>, rec, args..., std::make_pair(i, std::min(i + tile_size, size)));
          }
        } else {
          if (size == 0) {
            return;
          }
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

template<IterDirection tDirection, typename TRanges, typename TFun>
inline constexpr void iterate_tile(const auto& multi_size, const TRanges& ranges, TFun&& fun) {
  using Range = star::ElementType<TRanges>;
  using Size = star::ElementType<Range>;
  constexpr std::size_t dim_num = star::size<TRanges>;

  auto impl = [&](auto dim, auto rec, auto index, auto... args) THES_ALWAYS_INLINE {
    const auto indices = star::get_at<dim>(ranges);
    const auto begin = star::get_at<0>(indices);
    const auto end = star::get_at<1>(indices);

    if constexpr (dim + 1 < dim_num) {
      const auto factor = multi_size.template after_size<dim>();
      if constexpr (tDirection == IterDirection::FORWARD) {
        for (Size i = begin; i < end; ++i) {
          rec(index_tag<dim + 1>, rec, index + i * factor, args..., i);
        }
      } else {
        for (Size i = end; i > begin; --i) {
          rec(index_tag<dim + 1>, rec, index + (i - 1) * factor, args..., i - 1);
        }
      }
    } else {
      using IndexPos = IndexPosition<Size, dim_num>;
      if constexpr (tDirection == IterDirection::FORWARD) {
        for (Size i = begin; i < end; ++i) {
          fun(IndexPos{{args..., i}, index + i});
        }
      } else {
        for (Size i = end; i > begin; --i) {
          fun(IndexPos{{args..., i - 1}, index + (i - 1)});
        }
      }
    }
  };
  impl(index_tag<0>, impl, Size{0});
}
} // namespace thes

#endif // INCLUDE_THESAUROS_ALGORITHMS_RANGES_FOR_EACH_TILE_HPP
