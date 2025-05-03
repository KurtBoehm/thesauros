// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_ARRAY_LIMITED_HPP
#define INCLUDE_THESAUROS_CONTAINERS_ARRAY_LIMITED_HPP

#include <array>
#include <cassert>
#include <cstddef>

#include "thesauros/math/integer-cast.hpp"

namespace thes {
// Access to element beyond the size (up to the capacity) is allowed, but discouraged
template<typename T, std::size_t tCapacity>
struct LimitedArray {
  using Value = T;
  using value_type = Value;
  static constexpr std::size_t capacity = tCapacity;

  using Array = std::array<Value, capacity>;
  using const_iterator = const T*;

  constexpr LimitedArray() = default;
  explicit constexpr LimitedArray(std::size_t size) : size_(size) {
    assert(size_ <= capacity);
  }
  template<typename... Ts>
  requires(sizeof...(Ts) <= tCapacity)
  explicit constexpr LimitedArray(Ts... values) : size_(sizeof...(Ts)), data_{values...} {
    assert(size_ <= capacity);
  }

  template<typename TIt>
  LimitedArray(TIt first, TIt last) : size_(*safe_cast<std::size_t>(std::distance(first, last))) {
    assert(size_ <= capacity);
    std::copy(first, last, data_.begin());
  }

  constexpr const T& operator[](std::size_t idx) const {
    assert(idx < capacity);
    return data_[idx];
  }
  constexpr T& operator[](std::size_t idx) {
    assert(idx < capacity);
    return data_[idx];
  }

  [[nodiscard]] constexpr const T* begin() const {
    return data_.begin();
  }
  [[nodiscard]] constexpr const T* end() const {
    return data_.begin() + size_;
  }

  [[nodiscard]] constexpr const T* data() const {
    return data_.data();
  }
  [[nodiscard]] constexpr T* data() {
    return data_.data();
  }

  [[nodiscard]] constexpr const Array& as_array() const {
    return data_;
  }

  constexpr friend bool operator==(const LimitedArray& a1, const LimitedArray& a2) {
    return std::equal(a1.begin(), a1.end(), a2.begin(), a2.end());
  }

private:
  std::size_t size_{0};
  Array data_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_ARRAY_LIMITED_HPP
