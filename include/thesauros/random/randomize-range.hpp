// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_RANDOM_RANDOMIZE_RANGE_HPP
#define INCLUDE_THESAUROS_RANDOM_RANDOMIZE_RANGE_HPP

#include <bit>
#include <concepts>
#include <limits>
#include <random>

namespace thes {
template<std::unsigned_integral T>
struct RangeRandomizer {
  static constexpr auto mul = static_cast<T>(12605985483714917081ULL);
  static constexpr auto digits = std::numeric_limits<T>::digits;
  static constexpr T b1 = T{1} << (digits / 2);

  template<typename TGen>
  explicit RangeRandomizer(T size, TGen gen)
      : size_(size), offset_(std::uniform_int_distribution<T>{0, size}(gen)) {}

  [[nodiscard]] constexpr T size() const {
    return size_;
  }

  constexpr T transform(T x) const {
    auto part = [this](const T y) {
      const T splits = size_ & ~y;
      // y < size_ => split != 0
      const T m = (std::numeric_limits<T>::max() >> std::countl_zero(splits)) >> 1;

      const T z = (y ^ (y >> T{1})) * mul;
      return (y & ~m) | (z & m);
    };

    x = (x >= d_) ? (x - d_) : (x + offset_);
    x = cc_ - x;
    x = (x >= b_) ? (cc_ - x + b_) : x;
    x = cc_ - x;
    T y = x & aa_;
    x = ((y < c_) & (((y * mul) & b1) != 0)) ? (x ^ a_) : x;
    x = cc_ - x;
    x = part(x);
    // x = cc_ - x;
    // x = part(x);
    return x;
  }

private:
  T size_;
  T offset_;
  T a_ = std::bit_floor(size_);
  T aa_ = a_ - 1;
  T b_ = a_ - (size_ - a_);
  T cc_ = size_ - 1;
  T c_ = size_ & aa_;
  T d_ = size_ - offset_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_RANDOM_RANDOMIZE_RANGE_HPP
