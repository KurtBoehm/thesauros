#ifndef INCLUDE_THESAUROS_UTILITY_INDEX_POSITION_WRAPPER_HPP
#define INCLUDE_THESAUROS_UTILITY_INDEX_POSITION_WRAPPER_HPP

#include <array>
#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef>
#include <ostream>
#include <type_traits>

#include "thesauros/algorithms/static-ranges.hpp"
#include "thesauros/io/printers.hpp"
#include "thesauros/math.hpp"
#include "thesauros/utility/safe-cast.hpp"
#include "thesauros/utility/static-ranges.hpp"

namespace thes {
template<std::unsigned_integral TSize, std::size_t tDimNum>
struct IndexPositionWrapper {
  static constexpr std::size_t dimension_num = tDimNum;
  using Size = TSize;
  using SizeArr = std::array<Size, dimension_num>;
  using Div = thes::Divisor<Size>;
  using DivArr = std::array<Div, dimension_num>;
  using Diff = std::make_signed_t<Size>;

  constexpr IndexPositionWrapper(Size idx, SizeArr pos, SizeArr dims)
      : dims_(dims), pos_(pos), idx_(idx) {}
  constexpr IndexPositionWrapper(SizeArr pos, SizeArr dims)
      : dims_(dims), pos_(pos),
        idx_(star::position_to_index(pos, star::postfix_product_inclusive(dims))) {}
  constexpr IndexPositionWrapper(Size idx, SizeArr dims)
      : dims_(dims), pos_(star::index_to_position(idx, divs_)), idx_(idx) {}

  constexpr IndexPositionWrapper& operator++() {
    ++idx_;

    ++std::get<dimension_num - 1>(pos_);
    star::iota<1, dimension_num> | star::reversed | star::for_each([&](auto idx) {
      const auto pos_idx = std::get<idx>(pos_);

      const bool over = pos_idx == std::get<idx>(dims_);
      std::get<idx>(pos_) = over ? 0 : pos_idx;
      std::get<idx - 1>(pos_) += Size{over};
    });
    assert(std::get<0>(pos_) < std::get<0>(dims_));

    return *this;
  }
  constexpr IndexPositionWrapper& operator--() {
    --idx_;

    --std::get<dimension_num - 1>(pos_);
    star::iota<1, dimension_num> | star::reversed | star::for_each([&](auto idx) {
      const auto pos_idx = std::get<idx>(pos_);
      const auto dim_idx = std::get<idx>(dims_);

      const bool under = pos_idx == Size(-1);
      std::get<idx>(pos_) = under ? (dim_idx - 1) : pos_idx;
      std::get<idx - 1>(pos_) -= Size{under};
    });
    assert(std::get<0>(pos_) < std::get<0>(dims_));

    return *this;
  }

  constexpr IndexPositionWrapper& operator+=(Size off) {
    idx_ += off;
    pos_ = star::index_to_position(idx_, divs_);
    return *this;
  }
  friend constexpr IndexPositionWrapper operator+(IndexPositionWrapper w, Size off) {
    return w += off;
  }
  friend constexpr IndexPositionWrapper operator+(Size off, IndexPositionWrapper w) {
    return w += off;
  }
  constexpr IndexPositionWrapper& operator-=(Size off) {
    idx_ -= off;
    pos_ = star::index_to_position(idx_, divs_);
    return *this;
  }
  friend constexpr IndexPositionWrapper operator-(IndexPositionWrapper w, Size off) {
    return w -= off;
  }
  friend constexpr IndexPositionWrapper operator-(Size off, IndexPositionWrapper w) {
    return w -= off;
  }

  friend constexpr Diff operator-(const IndexPositionWrapper& w1, const IndexPositionWrapper& w2) {
    assert(w1.dims_ == w2.dims_);
    assert((w1.idx_ == w2.idx_) == (w1.pos_ == w2.pos_));
    return *safe_cast<Diff>(w1.idx_) - *safe_cast<Diff>(w2.idx_);
  }

  [[nodiscard]] constexpr Size index() const {
    return idx_;
  }
  [[nodiscard]] constexpr SizeArr position() const {
    return pos_;
  }

  constexpr friend bool operator==(const IndexPositionWrapper& w1, const IndexPositionWrapper& w2) {
    assert(w1.dims_ == w2.dims_);
    assert((w1.idx_ == w2.idx_) == (w1.pos_ == w2.pos_));
    return w1.idx_ == w2.idx_;
  }
  constexpr friend bool operator==(const IndexPositionWrapper& w, Size s) {
    return w.idx_ == s;
  }
  constexpr friend bool operator==(Size s, const IndexPositionWrapper& w) {
    return w.idx_ == s;
  }

  constexpr friend std::strong_ordering operator<=>(const IndexPositionWrapper& w1,
                                                    const IndexPositionWrapper& w2) {
    assert(w1.dims_ == w2.dims_);
    assert((w1.idx_ == w2.idx_) == (w1.pos_ == w2.pos_));
    return w1.idx_ <=> w2.idx_;
  }

  friend std::ostream& operator<<(std::ostream& stream, const IndexPositionWrapper& wrap) {
    return stream << wrap.idx_ << "@" << range_print(wrap);
  }

private:
  SizeArr dims_;
  DivArr divs_ = dims_ | star::transform([](auto dim) { return Div{dim}; }) | star::to_array;
  SizeArr pos_;
  Size idx_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_INDEX_POSITION_WRAPPER_HPP
