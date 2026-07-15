// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_ARRAY_LIMITED_HPP
#define INCLUDE_THESAUROS_CONTAINERS_ARRAY_LIMITED_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <utility>

#include "thesauros/math/integer-cast.hpp"

namespace thes {
/** A fixed-capacity array with a runtime size not exceeding `tCapacity`. */
template<typename V, std::size_t Capacity>
struct LimitedArray {
  using Value = V;
  using value_type = Value;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = Value&;
  using const_reference = const Value&;
  using pointer = Value*;
  using const_pointer = const Value*;
  using iterator = Value*;
  using const_iterator = const Value*;

  static constexpr std::size_t capacity = Capacity;

  using Array = std::array<Value, capacity>;

  constexpr LimitedArray() = default;
  explicit constexpr LimitedArray(std::size_t size) : size_(size) {
    assert(size_ <= capacity);
  }
  template<typename... Ts>
  requires(sizeof...(Ts) <= Capacity)
  explicit constexpr LimitedArray(Ts... values) : size_(sizeof...(Ts)), data_{values...} {
    assert(size_ <= capacity);
  }

  template<std::input_iterator TIt>
  constexpr LimitedArray(TIt first, TIt last)
      : size_(*safe_cast<std::size_t>(std::distance(first, last))) {
    assert(size_ <= capacity);
    std::ranges::copy(first, last, data_.begin());
  }

  [[nodiscard]] constexpr std::size_t size() const {
    return size_;
  }
  [[nodiscard]] constexpr bool empty() const {
    return size_ == 0;
  }

  constexpr decltype(auto) operator[](this auto&& self, std::size_t idx) {
    assert(idx < capacity);
    return self.data_[idx];
  }

  [[nodiscard]] constexpr auto begin(this auto&& self) {
    return self.data_.data();
  }
  [[nodiscard]] constexpr auto end(this auto&& self) {
    return self.data_.data() + self.size_;
  }

  [[nodiscard]] constexpr auto data(this auto&& self) {
    return self.data_.data();
  }

  [[nodiscard]] constexpr decltype(auto) front(this auto&& self) {
    assert(!self.empty());
    return (self.data_.front());
  }
  [[nodiscard]] constexpr decltype(auto) back(this auto&& self) {
    assert(!self.empty());
    return (self.data_[self.size_ - 1]);
  }

  constexpr void push_back(const Value& value) {
    assert(size_ < capacity);
    data_[size_] = value;
    ++size_;
  }
  constexpr void push_back(Value&& value) {
    assert(size_ < capacity);
    data_[size_] = std::move(value);
    ++size_;
  }
  constexpr void pop_back() {
    assert(!empty());
    --size_;
  }
  constexpr void clear() {
    size_ = 0;
  }

  [[nodiscard]] constexpr const Array& as_array() const {
    return data_;
  }

  [[nodiscard]] friend constexpr bool operator==(const LimitedArray& lhs, const LimitedArray& rhs) {
    return std::ranges::equal(lhs, rhs);
  }

  friend constexpr void swap(LimitedArray& lhs, LimitedArray& rhs) noexcept {
    using std::swap;
    swap(lhs.data_, rhs.data_);
    swap(lhs.size_, rhs.size_);
  }

private:
  std::size_t size_{0};
  Array data_{};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_ARRAY_LIMITED_HPP
