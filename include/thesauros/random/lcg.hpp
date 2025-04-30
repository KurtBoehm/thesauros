// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_RANDOM_LCG_HPP
#define INCLUDE_THESAUROS_RANDOM_LCG_HPP

#include <compare>
#include <cstddef>

#include "thesauros/iterator/facades.hpp"

namespace thes {
template<typename T>
struct LCG {
  explicit constexpr LCG(T seed, T increment, T size)
      : seed_(seed), increment_(increment), size_(size) {}

  struct IterProvider {
    using Value = T;
    using Diff = std::ptrdiff_t;

    using IterTypes = iter_provider::ValueTypes<Value, Diff>;

    static constexpr Value deref(const auto& self) {
      return self.value_;
    }
    static constexpr void incr(auto& self) {
      ++self.index_;
      self.value_ = added(self, self.value_, self.lcg_->increment_);
    }
    static constexpr void decr(auto& self) {
      --self.index_;
      self.value_ = added(self, self.value_, self.lcg_->size_ - self.lcg_->increment_);
    }
    static constexpr void iadd(auto& self, Diff diff) {
      self.index_ += static_cast<T>(diff);
      self.value_ = added(self, self.value_, skip_step(self, diff));
    }
    static constexpr bool eq(const auto& i1, const auto& i2) {
      return i1.index_ == i2.index_;
    }
    static constexpr std::strong_ordering three_way(const auto& i1, const auto& i2) {
      return i1.index_ <=> i2.index_;
    }
    static constexpr Diff sub(const auto& i1, const auto& i2) {
      return static_cast<Diff>(i1.index_) - static_cast<Diff>(i2.index_);
    }

  private:
    static constexpr T added(const auto& self, T value, T step) {
      const auto ref = self.lcg_->size_ - step;
      return (value < ref) ? (value + step) : (value - ref);
    }
    static constexpr T skip_step(const auto& self, Diff diff) {
      T step = (diff >= 0) ? self.lcg_->increment_ : (self.lcg_->size_ - self.lcg_->increment_);
      T num = (diff >= 0) ? static_cast<T>(diff) : -static_cast<T>(diff);
      T prod = 0;

      for (; num > 0; num >>= 1) {
        prod = (num & T{1}) ? added(self, prod, step) : prod;
        const T ref = self.lcg_->size_ - step;
        step = (step < ref) ? (2 * step) : (step - ref);
      }

      return prod;
    }
  };

  struct const_iterator : public IteratorFacade<const_iterator, IterProvider> {
    friend IterProvider;
    constexpr const_iterator(const LCG& lcg, T index, T value)
        : lcg_(&lcg), index_(index), value_(value) {}

    constexpr T index() const {
      return index_;
    }

  private:
    LCG const* lcg_;
    T index_;
    T value_;
  };

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
