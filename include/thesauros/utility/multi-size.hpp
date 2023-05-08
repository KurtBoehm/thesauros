#ifndef INCLUDE_THESAUROS_UTILITY_MULTI_SIZE_HPP
#define INCLUDE_THESAUROS_UTILITY_MULTI_SIZE_HPP

#include <array>
#include <cstddef>
#include <functional>

#include "thesauros/algorithms/static-ranges/index-to-position.hpp"
#include "thesauros/algorithms/static-ranges/position-to-index.hpp"
#include "thesauros/numerics/divmod.hpp"
#include "thesauros/utility/static-ranges/ranges/postfix-product-inclusive.hpp"
#include "thesauros/utility/static-ranges/ranges/transform.hpp"
#include "thesauros/utility/static-ranges/sinks/to-array.hpp"
#include "thesauros/utility/value-sequence.hpp"

namespace thes {
template<typename TIndex, std::size_t tDimNum>
struct BasicMultiSize {
  using Size = TIndex;
  static constexpr std::size_t dimension_num = tDimNum;
  using AxisSize = std::array<Size, dimension_num>;
  using ExAxisSize = std::array<Size, dimension_num + 1>;

  constexpr explicit BasicMultiSize(std::array<TIndex, tDimNum> sizes)
      : sizes_(sizes), postfix_prod_incl_(star::postfix_product_inclusive(sizes) | star::to_array) {
  }

  [[nodiscard]] constexpr const AxisSize& sizes() const {
    return sizes_;
  }
  [[nodiscard]] constexpr Size total_size() const {
    return star::get_at<0>(postfix_prod_incl_);
  }
  template<std::size_t tDim>
  [[nodiscard]] constexpr Size axis_size() const {
    return star::get_at<tDim>(sizes_);
  }

  [[nodiscard]] constexpr const ExAxisSize& from_sizes() const {
    return postfix_prod_incl_;
  }

  template<std::size_t tDim>
  [[nodiscard]] constexpr Size from_size() const {
    return star::get_at<tDim>(postfix_prod_incl_);
  }
  [[nodiscard]] constexpr Size from_size(std::size_t dim) const {
    postfix_prod_incl_[dim];
  }

  template<std::size_t tDim>
  [[nodiscard]] constexpr Size after_size() const {
    if constexpr (tDim + 1 == dimension_num) {
      return 1;
    } else {
      return from_size<tDim + 1>();
    }
  }
  [[nodiscard]] constexpr Size after_size(std::size_t dim) const {
    from_size(dim + 1);
  }

  [[nodiscard]] constexpr Size pos_to_index(AxisSize pos) const {
    return star::position_to_index(pos, postfix_prod_incl_);
  }

private:
  AxisSize sizes_;
  ExAxisSize postfix_prod_incl_;
};

template<typename TIndex, std::size_t tDimNum>
struct MultiSize : public BasicMultiSize<TIndex, tDimNum> {
  using Size = TIndex;
  static constexpr std::size_t dimension_num = tDimNum;
  using AxisSize = std::array<Size, dimension_num>;
  using ExAxisSize = std::array<Size, dimension_num + 1>;
  using Div = Divisor<Size>;
  using AxisDiv = std::array<Div, dimension_num>;
  using ExAxisDiv = std::array<Div, dimension_num + 1>;

  constexpr explicit MultiSize(std::array<TIndex, tDimNum> sizes)
      : BasicMultiSize<TIndex, tDimNum>(sizes),
        divs_(this->sizes() | star::transform([](Size a) { return Div(a); }) | star::to_array),
        postfix_prod_incl_divs_(this->from_sizes() |
                                star::transform([](Size a) { return Div(a); }) | star::to_array) {}

  [[nodiscard]] constexpr AxisSize index_to_pos(Size index) const {
    return star::index_to_position(index, divs_);
  }

  template<std::size_t tDim>
  [[nodiscard]] constexpr Size index_to_axis_index(Size index) const {
    if constexpr (tDim + 1 == dimension_num) {
      return index % star::get_at<tDim>(divs_);
    } else {
      return (index / star::get_at<tDim + 1>(postfix_prod_incl_divs_)) % star::get_at<tDim>(divs_);
    }
  }
  [[nodiscard]] constexpr Size index_to_axis_index(Size index, std::size_t dim) const {
    return (index / postfix_prod_incl_divs_[dim + 1]) % divs_[dim];
  }

  template<std::size_t tDim>
  const Div& from_size_div() const {
    return star::get_at<tDim>(postfix_prod_incl_divs_);
  }
  template<std::size_t tDim>
  const Div& after_size_div() const {
    return star::get_at<tDim + 1>(postfix_prod_incl_divs_);
  }

private:
  AxisDiv divs_;
  ExAxisDiv postfix_prod_incl_divs_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_MULTI_SIZE_HPP
