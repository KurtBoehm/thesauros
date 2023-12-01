#ifndef INCLUDE_THESAUROS_ALGORITHMS_RANGES_FOR_EACH_TILE_HPP
#define INCLUDE_THESAUROS_ALGORITHMS_RANGES_FOR_EACH_TILE_HPP

#include <array>
#include <cassert>
#include <cstddef>
#include <type_traits>

#include "thesauros/math/arithmetic.hpp"
#include "thesauros/utility/inlining.hpp"
#include "thesauros/utility/no-op.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/static-ranges/definitions/type-traits.hpp"
#include "thesauros/utility/type-tag.hpp"
#include "thesauros/utility/value-tag.hpp"

namespace thes {
enum struct IterDirection : bool { FORWARD, BACKWARD };

template<typename TIdx, typename TPos>
struct IndexPosition {
  using Index = TIdx;
  using Position = TPos;
  static constexpr std::size_t dimension_num = thes::star::size<Position>;

  explicit operator Index() const {
    return index;
  }

  friend decltype(auto) operator+(IndexPosition lhs, auto rhs)
  requires(requires { lhs.index + rhs; })
  {
    return lhs.index + rhs;
  }

  Index index;
  Position position;
};

template<typename>
struct AnyIndexPositionTrait : public std::false_type {};
template<typename TSize, typename TPos>
struct AnyIndexPositionTrait<IndexPosition<TSize, TPos>> : public std::true_type {};
template<typename T>
concept AnyIndexPosition = AnyIndexPositionTrait<T>::value;

// Split the hypercube described by “ranges” into tiles and iterate over them
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
          for (Size i = size_tile; i > 0; i -= tile_size) {
            const Size idx = i + begin;
            rec(index_tag<dim + 1>, rec, args...,
                std::make_pair(idx - tile_size, std::min(idx, end)));
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

// Iterate over the cells in a tile
template<IterDirection tDirection, typename TRanges,
         typename TIdx = star::Value<star::Value<TRanges>>>
inline constexpr void tile_for_each(const auto& multi_size, const TRanges& ranges, auto&& fun,
                                    TypeTag<TIdx> /*tag*/ = {}) {
  using Range = star::Value<TRanges>;
  using Size = star::Value<Range>;
  constexpr std::size_t dim_num = star::size<TRanges>;
  using IndexPos = IndexPosition<TIdx, std::array<Size, dim_num>>;

  auto impl = [&](auto dim, auto rec, auto index, auto... args) THES_ALWAYS_INLINE {
    const auto [begin, end] = star::get_at<dim>(ranges);

    if constexpr (tDirection == IterDirection::FORWARD) {
      for (Size i = begin; i < end; ++i) {
        if constexpr (dim + 1 < dim_num) {
          const auto factor = multi_size.after_size(dim);
          rec(index_tag<dim + 1>, rec, index + i * factor, args..., i);
        } else {
          fun(IndexPos{TIdx{index + i}, {args..., i}});
        }
      }
    } else {
      for (Size i = end; i > begin; --i) {
        if constexpr (dim + 1 < dim_num) {
          const auto factor = multi_size.after_size(dim);
          rec(index_tag<dim + 1>, rec, index + (i - 1) * factor, args..., i - 1);
        } else {
          fun(IndexPos{TIdx{index + (i - 1)}, {args..., i - 1}});
        }
      }
    }
  };
  impl(index_tag<0>, impl, Size{0});
}
template<IterDirection tDirection, typename TRanges,
         typename TIdx = star::Value<star::Value<TRanges>>>
inline constexpr void tile_for_each(const auto& multi_size, const TRanges& ranges, auto&& full_fun,
                                    auto&& part_fun, AnyIndexTag auto vec_size,
                                    AnyBoolTag auto has_part, TypeTag<TIdx> /*tag*/ = {}) {
  using Range = star::Value<TRanges>;
  using Size = star::Value<Range>;
  constexpr std::size_t dim_num = star::size<TRanges>;
  using IndexPos = IndexPosition<TIdx, std::array<Size, dim_num>>;

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
            full_fun(IndexPos{TIdx{index + i}, {args..., i}});
          }
          if (i != end) {
            part_fun(IndexPos{TIdx{index + i}, {args..., i}}, end - i);
          }
        } else {
          assert((end - begin) % vec_size == 0);
          for (Size i = begin; i < end; i += vec_size) {
            full_fun(IndexPos{TIdx{index + i}, {args..., i}});
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
            part_fun(IndexPos{TIdx{index + full_vec_end}, {args..., full_vec_end}},
                     end - full_vec_end);
          }
          for (Size i = full_vec_end; i > begin; i -= vec_size) {
            const Size idx = i - vec_size;
            full_fun(IndexPos{TIdx{index + idx}, {args..., idx}});
          }
        } else {
          assert((end - begin) % vec_size == 0);
          for (Size i = end; i > begin; i -= vec_size) {
            const Size idx = i - vec_size;
            full_fun(IndexPos{TIdx{index + idx}, {args..., idx}});
          }
        }
      }
    }
  };
  impl(index_tag<0>, impl, Size{0});
}

// Iterate over the elements described by “ranges” in a tiled fashion
template<IterDirection tDirection, typename TRanges, typename TFixedAxes,
         typename TIdx = star::Value<star::Value<TRanges>>>
inline constexpr void tiled_for_each(const auto& multi_size, const TRanges& ranges,
                                     const auto& tile_sizes, const TFixedAxes& fixed_axes,
                                     auto&& fun, TypeTag<TIdx> tag = {}) {
  thes::for_each_tile<tDirection>(ranges, tile_sizes, fixed_axes, [&](auto... args) {
    thes::tile_for_each<tDirection>(multi_size, std::array{args...}, fun, tag);
  });
}
template<IterDirection tDirection, typename TRanges, typename TFixedAxes,
         typename TIdx = star::Value<star::Value<TRanges>>>
inline constexpr void tiled_for_each(const auto& multi_size, const TRanges& ranges,
                                     const auto& tile_sizes, const TFixedAxes& fixed_axes,
                                     auto&& full_fun, auto&& part_fun, AnyIndexTag auto vec_size,
                                     TypeTag<TIdx> tag = {}) {
  thes::for_each_tile<tDirection>(
    ranges, tile_sizes, fixed_axes,
    /*full_fun=*/
    [&](auto... args) {
      thes::tile_for_each<tDirection>(multi_size, std::array{args...}, full_fun, thes::NoOp{},
                                      vec_size, /*has_part=*/thes::false_tag, tag);
    },
    /*part_fun=*/
    [&](auto... args) {
      thes::tile_for_each<tDirection>(multi_size, std::array{args...}, full_fun, part_fun, vec_size,
                                      /*has_part=*/thes::true_tag, tag);
    },
    vec_size);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_ALGORITHMS_RANGES_FOR_EACH_TILE_HPP
