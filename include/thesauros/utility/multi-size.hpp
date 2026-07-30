// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_MULTI_SIZE_HPP
#define INCLUDE_THESAUROS_UTILITY_MULTI_SIZE_HPP

#include <array>
#include <cstddef>

#include "thesauros/algorithms/static-ranges/index-to-position.hpp"
#include "thesauros/algorithms/static-ranges/position-to-index.hpp"
#include "thesauros/math/divmod.hpp"
#include "thesauros/ranges/indices.hpp"
#include "thesauros/static-ranges/definitions/static-apply.hpp"
#include "thesauros/static-ranges/piping.hpp" // IWYU pragma: keep
#include "thesauros/static-ranges/sinks/postfix-product-inclusive.hpp"
#include "thesauros/static-ranges/sinks/to-array.hpp"
#include "thesauros/static-ranges/views/transform.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes {
template<typename S, std::size_t N>
struct BasicMultiSize {
  using Size = S;
  static constexpr std::size_t dimension_num = N;
  using AxisSize = std::array<Size, dimension_num>;
  using ExAxisSize = std::array<Size, dimension_num + 1>;

  explicit constexpr BasicMultiSize(std::array<S, N> sizes)
      : sizes_(sizes), postfix_prod_incl_(star::postfix_product_inclusive(sizes) | star::to_array) {
  }

  [[nodiscard]] constexpr const AxisSize& sizes() const {
    return sizes_;
  }
  [[nodiscard]] constexpr Size total_size() const {
    return std::get<0>(postfix_prod_incl_);
  }
  template<std::size_t Dim>
  [[nodiscard]] constexpr Size axis_size(IndexTag<Dim> /*tag*/ = {}) const {
    return std::get<Dim>(sizes_);
  }
  [[nodiscard]] constexpr Size axis_size(std::size_t dim) const {
    return sizes_[dim];
  }

  [[nodiscard]] constexpr const ExAxisSize& from_sizes() const {
    return postfix_prod_incl_;
  }

  template<std::size_t Dim>
  [[nodiscard]] constexpr Size from_size(IndexTag<Dim> /*tag*/ = {}) const {
    return std::get<Dim>(postfix_prod_incl_);
  }
  [[nodiscard]] constexpr Size from_size(std::size_t dim) const {
    return postfix_prod_incl_[dim];
  }

  template<std::size_t Dim>
  [[nodiscard]] constexpr Size after_size(IndexTag<Dim> /*tag*/ = {}) const {
    if constexpr (Dim + 1 == dimension_num) {
      return 1;
    } else {
      return from_size<Dim + 1>();
    }
  }
  [[nodiscard]] constexpr Size after_size(std::size_t dim) const {
    return from_size(dim + 1);
  }

  [[nodiscard]] constexpr Size pos_to_index(AxisSize pos) const {
    return star::position_to_index(pos, postfix_prod_incl_);
  }

  bool operator==(const BasicMultiSize&) const = default;

private:
  AxisSize sizes_;
  ExAxisSize postfix_prod_incl_;
};

template<typename S, std::size_t N>
struct MultiSize : public BasicMultiSize<S, N> {
  using Size = S;
  static constexpr std::size_t dimension_num = N;
  using AxisSize = std::array<Size, dimension_num>;
  using ExAxisSize = std::array<Size, dimension_num + 1>;
  using Div = Divisor<Size>;
  using AxisDiv = std::array<Div, dimension_num>;
  using ExAxisDiv = std::array<Div, dimension_num + 1>;

  explicit constexpr MultiSize(std::array<S, N> sizes)
      : BasicMultiSize<S, N>(sizes),
        divs_(this->sizes() | star::transform([](Size a) { return Div(a); }) | star::to_array),
        postfix_prod_incl_divs_(this->from_sizes() |
                                star::transform([](Size a) { return Div(a); }) | star::to_array) {}

  [[nodiscard]] constexpr AxisSize index_to_pos(Size index) const {
    return star::index_to_position(index, divs_);
  }

  template<std::size_t Dim>
  [[nodiscard]] constexpr Size index_to_axis_index(Size index, IndexTag<Dim> /*tag*/ = {}) const {
    if constexpr (Dim + 1 == dimension_num) {
      return index % std::get<Dim>(divs_);
    } else {
      return (index / std::get<Dim + 1>(postfix_prod_incl_divs_)) % std::get<Dim>(divs_);
    }
  }
  [[nodiscard]] constexpr Size index_to_axis_index(Size index, std::size_t dim) const {
    return (index / postfix_prod_incl_divs_[dim + 1]) % divs_[dim];
  }

  template<std::size_t Dim>
  [[nodiscard]] const Div& from_size_div(IndexTag<Dim> /*tag*/ = {}) const {
    return std::get<Dim>(postfix_prod_incl_divs_);
  }
  template<std::size_t Dim>
  [[nodiscard]] const Div& after_size_div(IndexTag<Dim> /*tag*/ = {}) const {
    return std::get<Dim + 1>(postfix_prod_incl_divs_);
  }

private:
  AxisDiv divs_;
  ExAxisDiv postfix_prod_incl_divs_;
};

template<typename S, std::size_t N>
struct SubMultiSize {
  using Size = S;
  static constexpr std::size_t dimension_num = N;
  using AxisSize = std::array<Size, dimension_num>;
  using ExAxisSize = std::array<Size, dimension_num + 1>;

  constexpr SubMultiSize(std::array<S, N> offsets, std::array<S, N> sizes)
      : offsets_(offsets), ms_(sizes) {}

  [[nodiscard]] MultiSize<S, N> multisize() const {
    return ms_;
  }

  [[nodiscard]] AxisSize begins() const {
    return offsets_;
  }
  template<std::size_t Dim>
  [[nodiscard]] Size axis_begin(IndexTag<Dim> /*tag*/ = {}) const {
    return std::get<Dim>(offsets_);
  }
  [[nodiscard]] Size axis_begin(std::size_t dim) const {
    return offsets_[dim];
  }

  [[nodiscard]] AxisSize ends() const {
    return star::index_transform<dimension_num>([&](auto i) { return axis_end(i); }) |
           star::to_array;
  }
  template<std::size_t Dim>
  [[nodiscard]] Size axis_end(IndexTag<Dim> dim = {}) const {
    return std::get<Dim>(offsets_) + ms_.axis_size(dim);
  }
  [[nodiscard]] Size axis_end(std::size_t dim) const {
    return offsets_[dim] + ms_.axis_size(dim);
  }

  [[nodiscard]] AxisSize sizes() const {
    return ms_.sizes();
  }
  [[nodiscard]] constexpr Size total_size() const {
    return ms_.total_size();
  }
  template<std::size_t Dim>
  [[nodiscard]] Size axis_size(IndexTag<Dim> dim = {}) const {
    return ms_.axis_size(dim);
  }
  [[nodiscard]] Size axis_size(std::size_t dim) const {
    return ms_.axis_size(dim);
  }

  [[nodiscard]] std::array<ranges::IotaRange<Size>, dimension_num> axis_ranges() const {
    return star::static_apply<dimension_num>(
      [&]<std::size_t... I>() { return std::array{axis_range<I>()...}; });
  }
  template<std::size_t Dim>
  [[nodiscard]] ranges::IotaRange<Size> axis_range(IndexTag<Dim> dim = {}) const {
    return views::indices(axis_begin(dim), axis_end(dim));
  }
  [[nodiscard]] ranges::IotaRange<Size> axis_range(std::size_t dim) const {
    return views::indices(axis_begin(dim), axis_end(dim));
  }

  [[nodiscard]] AxisSize local_to_global_pos(AxisSize pos) const {
    return star::static_apply<dimension_num>(
      [&]<std::size_t... I>() { return std::array{(get<I>(pos) + axis_begin<I>())...}; });
  }
  [[nodiscard]] AxisSize global_to_local_pos(AxisSize pos) const {
    return star::static_apply<dimension_num>(
      [&]<std::size_t... I>() { return std::array{(get<I>(pos) - axis_begin<I>())...}; });
  }

  [[nodiscard]] Size local_pos_to_index(AxisSize pos) const {
    return ms_.pos_to_index(pos);
  }
  [[nodiscard]] Size global_pos_to_local_index(AxisSize pos) const {
    return local_pos_to_index(global_to_local_pos(pos));
  }

  [[nodiscard]] AxisSize local_index_to_pos(Size i) const {
    return ms_.index_to_pos(i);
  }
  [[nodiscard]] AxisSize local_index_to_global_pos(Size i) const {
    return local_to_global_pos(ms_.index_to_pos(i));
  }

  [[nodiscard]] SubMultiSize expand(Size amount,
                                    const MultiSize<Size, dimension_num>& world) const {
    const auto offsets = star::static_apply<dimension_num>([&]<std::size_t... I>() {
      return AxisSize{sub_min(std::get<I>(offsets_), amount, Size{})...};
    });
    const auto sizes = star::static_apply<dimension_num>([&]<std::size_t... I>() {
      return AxisSize{
        (add_max(axis_end<I>(), amount, world.axis_size(index_tag<I>)) - std::get<I>(offsets))...};
    });
    return {offsets, sizes};
  }

  [[nodiscard]] AxisSize reflect_into(AxisSize pos) const {
    auto f = [&](auto i) {
      const auto coord = get<i>(pos);
      const auto i0 = get<i>(offsets_);
      const auto i1 = i0 + ms_.axis_size(i);
      if (coord < i0) [[unlikely]] {
        return i0 + (i0 - coord - 1);
      }
      if (coord >= i1) [[unlikely]] {
        return i1 - (coord - i1 + 1);
      }
      return coord;
    };
    return star::static_apply<dimension_num>(
      [&]<std::size_t... I>() { return std::array{f(index_tag<I>)...}; });
  }
  // reflects the axes selected, where contains(pos) is assumed to be true
  [[nodiscard]] AxisSize reflect(AxisSize pos, std::array<bool, dimension_num> selectors) const {
    auto f = [&](auto i) {
      const auto coord = get<i>(pos);
      const auto i0 = get<i>(offsets_);
      const auto i1 = i0 + ms_.axis_size(i);
      const auto b = get<i>(selectors);
      return b ? (i1 - (coord - i0 + 1)) : coord;
    };
    return star::static_apply<dimension_num>(
      [&]<std::size_t... I>() { return std::array{f(index_tag<I>)...}; });
  }

  [[nodiscard]] bool contains(AxisSize pos) const {
    return star::static_apply<dimension_num>(
      [&]<std::size_t... I>() { return (... && axis_range<I>().contains(std::get<I>(pos))); });
  }

private:
  AxisSize offsets_;
  MultiSize<S, N> ms_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_MULTI_SIZE_HPP
