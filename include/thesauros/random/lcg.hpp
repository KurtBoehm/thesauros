// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_RANDOM_LCG_HPP
#define INCLUDE_THESAUROS_RANDOM_LCG_HPP

#include <compare>
#include <cstddef>

#include "thesauros/iterator/facade.hpp"

namespace thes {
template<typename T>
struct LCG {
  explicit constexpr LCG(T seed, T increment, T size)
      : seed_(seed), increment_(increment), size_(size) {}

  struct ConstIterator : public IteratorFacade<iter::ValueTypes<T, std::ptrdiff_t>> {
    using Diff = std::ptrdiff_t;

    friend IteratorFacade<iter::ValueTypes<T, Diff>>;

    // Required for `std::sentinel_for`, and hence for `LCG` to model `std::ranges::range`.
    constexpr ConstIterator() = default;

    constexpr ConstIterator(const LCG& lcg, T index, T value)
        : lcg_(&lcg), index_(index), value_(value) {}

    constexpr T index() const {
      return index_;
    }

  private:
    constexpr T deref() const {
      return value_;
    }
    constexpr void incr() {
      ++index_;
      value_ = added(value_, lcg_->increment_);
    }
    constexpr void decr() {
      --index_;
      value_ = added(value_, static_cast<T>(lcg_->size_ - lcg_->increment_));
    }
    constexpr void iadd(Diff diff) {
      index_ += static_cast<T>(diff);
      value_ = added(value_, skip_step(diff));
    }
    constexpr bool eq(const ConstIterator& other) const {
      return index_ == other.index_;
    }
    constexpr std::strong_ordering three_way(const ConstIterator& other) const {
      return index_ <=> other.index_;
    }
    constexpr Diff sub(const ConstIterator& other) const {
      return static_cast<Diff>(index_) - static_cast<Diff>(other.index_);
    }

    /** Adds `step < size` to `value < size` modulo `size`, without ever overflowing `T`. */
    constexpr T added(T value, T step) const {
      const auto ref = static_cast<T>(lcg_->size_ - step);
      return (value < ref) ? static_cast<T>(value + step) : static_cast<T>(value - ref);
    }
    /** The total step for a jump of `diff` elements, by doubling the per-element step. */
    constexpr T skip_step(Diff diff) const {
      const bool forward = diff >= 0;
      T step = forward ? lcg_->increment_ : static_cast<T>(lcg_->size_ - lcg_->increment_);
      T num = forward ? static_cast<T>(diff) : static_cast<T>(-static_cast<T>(diff));
      T prod = 0;

      for (; num > 0; num = static_cast<T>(num >> 1U)) {
        prod = (num & T{1}) ? added(prod, step) : prod;
        const auto ref = static_cast<T>(lcg_->size_ - step);
        step = (step < ref) ? static_cast<T>(2 * step) : static_cast<T>(step - ref);
      }

      return prod;
    }

    LCG const* lcg_{};
    T index_{};
    T value_{};
  };

  using const_iterator = ConstIterator;

  [[nodiscard]] constexpr T seed() const {
    return seed_;
  }
  [[nodiscard]] constexpr T increment() const {
    return increment_;
  }

  const_iterator begin() const {
    return {*this, 0, seed_};
  }
  const_iterator end() const {
    return {*this, size_, seed_};
  }

private:
  T seed_;
  T increment_;
  T size_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_RANDOM_LCG_HPP
