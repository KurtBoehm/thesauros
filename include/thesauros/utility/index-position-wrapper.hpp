// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_INDEX_POSITION_WRAPPER_HPP
#define INCLUDE_THESAUROS_UTILITY_INDEX_POSITION_WRAPPER_HPP

#include <array>
#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef>
#include <functional>
#include <type_traits>

#include "thesauros/algorithms/static-ranges/index-to-position.hpp"
#include "thesauros/algorithms/static-ranges/position-to-index.hpp"
#include "thesauros/math/divmod.hpp"
#include "thesauros/math/integer-cast.hpp"
#include "thesauros/static-ranges/piping.hpp" // IWYU pragma: keep
#include "thesauros/static-ranges/sinks/for-each.hpp"
#include "thesauros/static-ranges/sinks/postfix-product-inclusive.hpp"
#include "thesauros/static-ranges/sinks/reduce.hpp"
#include "thesauros/static-ranges/sinks/to-array.hpp"
#include "thesauros/static-ranges/views/iota.hpp"
#include "thesauros/static-ranges/views/reversed.hpp"
#include "thesauros/static-ranges/views/transform.hpp"

namespace thes {
namespace detail {
template<std::unsigned_integral S>
struct SingleIndexManager {
  using Size = S;
  using IdxSize = Size;
  using PosSize = Size;

  explicit constexpr SingleIndexManager(Size idx) : idx_(idx) {}

  constexpr Size index() const {
    return idx_;
  }
  constexpr Size pos_index() const {
    return idx_;
  }

  constexpr void incr() {
    ++idx_;
  }
  constexpr void decr() {
    --idx_;
  }
  constexpr void add(Size off) {
    idx_ += off;
  }
  constexpr void sub(Size off) {
    idx_ -= off;
  }

  constexpr bool operator==(const SingleIndexManager&) const = default;

private:
  Size idx_;
};

template<std::unsigned_integral IdxS, std::unsigned_integral PosS>
struct DualIndexManager {
  static_assert(sizeof(IdxS) <= sizeof(PosS));
  using IdxSize = IdxS;
  using PosSize = PosS;

  constexpr DualIndexManager(IdxSize idx, PosSize pos_idx) : idx_(idx), pos_idx_(pos_idx) {}

  constexpr IdxSize index() const {
    return idx_;
  }
  constexpr PosSize pos_index() const {
    return pos_idx_;
  }

  constexpr void incr() {
    ++idx_;
    ++pos_idx_;
  }
  constexpr void decr() {
    --idx_;
    --pos_idx_;
  }
  constexpr void add(IdxSize off) {
    idx_ += off;
    pos_idx_ += off;
  }
  constexpr void sub(IdxSize off) {
    idx_ -= off;
    pos_idx_ -= off;
  }

  constexpr bool operator==(const DualIndexManager& other) const {
    assert((idx_ == other.idx_) == (pos_idx_ == other.pos_idx_));
    return idx_ == other.idx_;
  }

private:
  IdxSize idx_;
  PosSize pos_idx_;
};

template<std::unsigned_integral T, std::size_t N>
constexpr auto dims_to_divs(std::array<T, N> dims) {
  return dims | star::transform([](auto dim) { return Divisor<T>{dim}; }) | star::to_array;
}

template<typename Derived, typename IdxMan, std::size_t DimN>
struct BasePosIndexWrapper {
  static constexpr std::size_t dimension_num = DimN;
  using IndexManager = IdxMan;
  using IdxSize = IndexManager::IdxSize;
  using PosSize = IndexManager::PosSize;

  using SizeArr = std::array<PosSize, dimension_num>;
  using DivArr = std::array<Divisor<PosSize>, dimension_num>;
  using Diff = std::make_signed_t<IdxSize>;

  constexpr BasePosIndexWrapper(IndexManager idx_man, SizeArr pos, SizeArr dims)
      : dims_(dims), pos_(pos), idx_man_(idx_man) {}
  constexpr BasePosIndexWrapper(IndexManager idx_man, SizeArr pos, SizeArr dims, DivArr divs)
      : dims_(dims), divs_(divs), pos_(pos), idx_man_(idx_man) {}

  constexpr Derived& operator++() {
    idx_man_.incr();

    ++std::get<dimension_num - 1>(pos_);
    star::tagged_iota<1, dimension_num> | star::reversed | star::for_each([&](auto idx) {
      const auto pos_idx = std::get<idx>(pos_);

      const bool over = pos_idx == std::get<idx>(dims_);
      std::get<idx>(pos_) = over ? 0 : pos_idx;
      std::get<idx - 1>(pos_) += PosSize{over};
    });
    assert(idx_man_.pos_index() <= (dims_ | star::left_reduce(std::multiplies{})));
    assert(star::index_to_position(idx_man_.pos_index(), divs_) == pos_);

    return der();
  }
  constexpr Derived& operator--() {
    idx_man_.decr();

    --std::get<dimension_num - 1>(pos_);
    star::tagged_iota<1, dimension_num> | star::reversed | star::for_each([&](auto idx) {
      const auto pos_idx = std::get<idx>(pos_);
      const auto dim_idx = std::get<idx>(dims_);

      const bool under = pos_idx == PosSize(-1);
      std::get<idx>(pos_) = under ? (dim_idx - 1) : pos_idx;
      std::get<idx - 1>(pos_) -= PosSize{under};
    });
    assert(std::get<0>(pos_) < std::get<0>(dims_));
    assert(star::index_to_position(idx_man_.pos_index(), divs_) == pos_);

    return der();
  }

  constexpr Derived& operator+=(IdxSize off) {
    idx_man_.add(off);
    pos_ = star::index_to_position(idx_man_.pos_index(), divs_);
    return der();
  }
  friend constexpr Derived operator+(Derived w, IdxSize off) {
    return w += off;
  }
  friend constexpr Derived operator+(IdxSize off, Derived w) {
    return w += off;
  }
  constexpr Derived& operator-=(IdxSize off) {
    idx_man_.sub(off);
    pos_ = star::index_to_position(idx_man_.pos_index(), divs_);
    return der();
  }
  friend constexpr Derived operator-(Derived w, IdxSize off) {
    return w -= off;
  }
  friend constexpr Derived operator-(IdxSize off, Derived w) {
    return w -= off;
  }

  friend constexpr Diff operator-(const Derived& w1, const Derived& w2) {
    assert(w1.dims_ == w2.dims_);
    assert((w1.idx_man_ == w2.idx_man_) == (w1.pos_ == w2.pos_));
    return *safe_cast<Diff>(w1.index()) - *safe_cast<Diff>(w2.index());
  }

  [[nodiscard]] constexpr IdxSize index() const {
    return idx_man_.index();
  }
  [[nodiscard]] constexpr SizeArr position() const {
    return pos_;
  }

  constexpr friend bool operator==(const Derived& w1, const Derived& w2) {
    assert(w1.dims_ == w2.dims_);
    assert((w1.idx_man_ == w2.idx_man_) == (w1.pos_ == w2.pos_));
    return w1.index() == w2.index();
  }
  constexpr friend bool operator==(const Derived& w, IdxSize s) {
    return w.index() == s;
  }
  constexpr friend bool operator==(IdxSize s, const Derived& w) {
    return w.index() == s;
  }

  constexpr friend std::strong_ordering operator<=>(const Derived& w1, const Derived& w2) {
    assert(w1.dims_ == w2.dims_);
    assert((w1.idx_man_ == w2.idx_man_) == (w1.pos_ == w2.pos_));
    return w1.index() <=> w2.index();
  }

private:
  constexpr Derived& der() {
    return static_cast<Derived&>(*this);
  }

  SizeArr dims_;
  DivArr divs_ = dims_to_divs(dims_);
  SizeArr pos_;
  IdxMan idx_man_;
};
} // namespace detail

template<std::unsigned_integral S, std::size_t DimN>
struct PosIndexWrapper : public detail::BasePosIndexWrapper<PosIndexWrapper<S, DimN>,
                                                            detail::SingleIndexManager<S>, DimN> {
  using IdxMan = detail::SingleIndexManager<S>;
  using Base = detail::BasePosIndexWrapper<PosIndexWrapper, IdxMan, DimN>;
  using SizeArr = std::array<S, DimN>;

  constexpr PosIndexWrapper(S idx, SizeArr pos, SizeArr dims) : Base(IdxMan{idx}, pos, dims) {}
  constexpr PosIndexWrapper(SizeArr pos, SizeArr dims)
      : Base(IdxMan{star::position_to_index(pos, star::postfix_product_inclusive(dims))}, pos,
             dims) {}
  constexpr PosIndexWrapper(S idx, SizeArr dims)
      : PosIndexWrapper(idx, dims, detail::dims_to_divs(dims)) {}

private:
  constexpr PosIndexWrapper(S idx, SizeArr dims, auto divs)
      : Base(IdxMan{idx}, star::index_to_position(idx, divs), dims, divs) {}
};

template<std::unsigned_integral IdxS, std::unsigned_integral PosS, std::size_t DimN>
struct DualPosIndexWrapper
    : public detail::BasePosIndexWrapper<DualPosIndexWrapper<IdxS, PosS, DimN>,
                                         detail::DualIndexManager<IdxS, PosS>, DimN> {
  using IdxMan = detail::DualIndexManager<IdxS, PosS>;
  using Base = detail::BasePosIndexWrapper<DualPosIndexWrapper, IdxMan, DimN>;
  using SizeArr = std::array<PosS, DimN>;

  constexpr DualPosIndexWrapper(IdxS idx, PosS pos_idx, SizeArr pos, SizeArr dims)
      : Base(IdxMan{idx, pos_idx}, pos, dims) {}
  constexpr DualPosIndexWrapper(IdxS idx, SizeArr pos, SizeArr dims)
      : Base(IdxMan{idx, star::position_to_index(pos, star::postfix_product_inclusive(dims))}, pos,
             dims) {}
  constexpr DualPosIndexWrapper(IdxS idx, PosS pos_idx, SizeArr dims)
      : DualPosIndexWrapper(idx, pos_idx, dims, detail::dims_to_divs(dims)) {}

private:
  constexpr DualPosIndexWrapper(IdxS idx, PosS pos_idx, SizeArr dims, auto divs)
      : Base(IdxMan{idx, pos_idx}, star::index_to_position(pos_idx, divs), dims, divs) {}
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_INDEX_POSITION_WRAPPER_HPP
