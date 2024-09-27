#ifndef INCLUDE_THESAUROS_UTILITY_INDEX_POSITION_WRAPPER_HPP
#define INCLUDE_THESAUROS_UTILITY_INDEX_POSITION_WRAPPER_HPP

#include <array>
#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef>
#include <functional>
#include <ostream>
#include <type_traits>

#include "thesauros/algorithms/static-ranges.hpp"
#include "thesauros/io/printers.hpp"
#include "thesauros/math.hpp"
#include "thesauros/utility/integer-cast.hpp"
#include "thesauros/utility/static-ranges.hpp"

namespace thes {
namespace detail {
template<std::unsigned_integral TSize>
struct SingleIndexManager {
  using Size = TSize;
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

template<std::unsigned_integral TIdxSize, std::unsigned_integral TPosSize>
struct DualIndexManager {
  static_assert(sizeof(TIdxSize) <= sizeof(TPosSize));
  using IdxSize = TIdxSize;
  using PosSize = TPosSize;

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

template<std::unsigned_integral T, std::size_t tSize>
inline constexpr auto dims_to_divs(std::array<T, tSize> dims) {
  return dims | star::transform([](auto dim) { return Divisor<T>{dim}; }) | star::to_array;
}

template<typename TDerived, typename TIdxMan, std::size_t tDimNum>
struct BasePosIndexWrapper {
  static constexpr std::size_t dimension_num = tDimNum;
  using IndexManager = TIdxMan;
  using IdxSize = IndexManager::IdxSize;
  using PosSize = IndexManager::PosSize;

  using SizeArr = std::array<PosSize, dimension_num>;
  using DivArr = std::array<Divisor<PosSize>, dimension_num>;
  using Diff = std::make_signed_t<IdxSize>;

  constexpr BasePosIndexWrapper(IndexManager idx_man, SizeArr pos, SizeArr dims)
      : dims_(dims), pos_(pos), idx_man_(idx_man) {}
  constexpr BasePosIndexWrapper(IndexManager idx_man, SizeArr pos, SizeArr dims, DivArr divs)
      : dims_(dims), divs_(divs), pos_(pos), idx_man_(idx_man) {}

  constexpr TDerived& operator++() {
    idx_man_.incr();

    ++std::get<dimension_num - 1>(pos_);
    star::iota<1, dimension_num> | star::reversed | star::for_each([&](auto idx) {
      const auto pos_idx = std::get<idx>(pos_);

      const bool over = pos_idx == std::get<idx>(dims_);
      std::get<idx>(pos_) = over ? 0 : pos_idx;
      std::get<idx - 1>(pos_) += PosSize{over};
    });
    assert(idx_man_.pos_index() <= (dims_ | star::left_reduce(std::multiplies{})));
    assert(star::index_to_position(idx_man_.pos_index(), divs_) == pos_);

    return der();
  }
  constexpr TDerived& operator--() {
    idx_man_.decr();

    --std::get<dimension_num - 1>(pos_);
    star::iota<1, dimension_num> | star::reversed | star::for_each([&](auto idx) {
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

  constexpr TDerived& operator+=(IdxSize off) {
    idx_man_.add(off);
    pos_ = star::index_to_position(idx_man_.pos_index(), divs_);
    return der();
  }
  friend constexpr TDerived operator+(TDerived w, IdxSize off) {
    return w += off;
  }
  friend constexpr TDerived operator+(IdxSize off, TDerived w) {
    return w += off;
  }
  constexpr TDerived& operator-=(IdxSize off) {
    idx_man_.sub(off);
    pos_ = star::index_to_position(idx_man_.pos_index(), divs_);
    return der();
  }
  friend constexpr TDerived operator-(TDerived w, IdxSize off) {
    return w -= off;
  }
  friend constexpr TDerived operator-(IdxSize off, TDerived w) {
    return w -= off;
  }

  friend constexpr Diff operator-(const TDerived& w1, const TDerived& w2) {
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

  constexpr friend bool operator==(const TDerived& w1, const TDerived& w2) {
    assert(w1.dims_ == w2.dims_);
    assert((w1.idx_man_ == w2.idx_man_) == (w1.pos_ == w2.pos_));
    return w1.index() == w2.index();
  }
  constexpr friend bool operator==(const TDerived& w, IdxSize s) {
    return w.index() == s;
  }
  constexpr friend bool operator==(IdxSize s, const TDerived& w) {
    return w.index() == s;
  }

  constexpr friend std::strong_ordering operator<=>(const TDerived& w1, const TDerived& w2) {
    assert(w1.dims_ == w2.dims_);
    assert((w1.idx_man_ == w2.idx_man_) == (w1.pos_ == w2.pos_));
    return w1.index() <=> w2.index();
  }

  friend std::ostream& operator<<(std::ostream& stream, const TDerived& wrap) {
    return stream << wrap.index() << "@" << range_print(wrap);
  }

private:
  constexpr TDerived& der() {
    return static_cast<TDerived&>(*this);
  }

  SizeArr dims_;
  DivArr divs_ = dims_to_divs(dims_);
  SizeArr pos_;
  TIdxMan idx_man_;
};
} // namespace detail

template<std::unsigned_integral TSize, std::size_t tDimNum>
struct PosIndexWrapper
    : public detail::BasePosIndexWrapper<PosIndexWrapper<TSize, tDimNum>,
                                         detail::SingleIndexManager<TSize>, tDimNum> {
  using IdxMan = detail::SingleIndexManager<TSize>;
  using Base = detail::BasePosIndexWrapper<PosIndexWrapper, IdxMan, tDimNum>;
  using SizeArr = std::array<TSize, tDimNum>;

  constexpr PosIndexWrapper(TSize idx, SizeArr pos, SizeArr dims) : Base(IdxMan{idx}, pos, dims) {}
  constexpr PosIndexWrapper(SizeArr pos, SizeArr dims)
      : Base(IdxMan{star::position_to_index(pos, star::postfix_product_inclusive(dims))}, pos,
             dims) {}
  constexpr PosIndexWrapper(TSize idx, SizeArr dims)
      : PosIndexWrapper(idx, dims, detail::dims_to_divs(dims)) {}

private:
  constexpr PosIndexWrapper(TSize idx, SizeArr dims, auto divs)
      : Base(IdxMan{idx}, star::index_to_position(idx, divs), dims, divs) {}
};

template<std::unsigned_integral TIdxSize, std::unsigned_integral TPosSize, std::size_t tDimNum>
struct DualPosIndexWrapper
    : public detail::BasePosIndexWrapper<DualPosIndexWrapper<TIdxSize, TPosSize, tDimNum>,
                                         detail::DualIndexManager<TIdxSize, TPosSize>, tDimNum> {
  using IdxMan = detail::DualIndexManager<TIdxSize, TPosSize>;
  using Base = detail::BasePosIndexWrapper<DualPosIndexWrapper, IdxMan, tDimNum>;
  using SizeArr = std::array<TPosSize, tDimNum>;

  constexpr DualPosIndexWrapper(TIdxSize idx, TPosSize pos_idx, SizeArr pos, SizeArr dims)
      : Base(IdxMan{idx, pos_idx}, pos, dims) {}
  constexpr DualPosIndexWrapper(TIdxSize idx, SizeArr pos, SizeArr dims)
      : Base(IdxMan{idx, star::position_to_index(pos, star::postfix_product_inclusive(dims))}, pos,
             dims) {}
  constexpr DualPosIndexWrapper(TIdxSize idx, TPosSize pos_idx, SizeArr dims)
      : DualPosIndexWrapper(idx, pos_idx, dims, detail::dims_to_divs(dims)) {}

private:
  constexpr DualPosIndexWrapper(TIdxSize idx, TPosSize pos_idx, SizeArr dims, auto divs)
      : Base(IdxMan{idx, pos_idx}, star::index_to_position(pos_idx, divs), dims, divs) {}
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_INDEX_POSITION_WRAPPER_HPP
