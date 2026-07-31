// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_ARRAY_FIXED_HPP
#define INCLUDE_THESAUROS_CONTAINERS_ARRAY_FIXED_HPP

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>

#include "thesauros/charconv/concat.hpp"
#include "thesauros/containers/array/construction.hpp"
#include "thesauros/containers/array/initialization-policy.hpp"
#include "thesauros/containers/array/typed-chunk.hpp"

namespace thes {
template<typename V, typename InitPol = DefaultInit, typename Alloc = std::allocator<V>>
struct FixedArray {
  using Data = TypedChunk<V, std::size_t, Alloc>;

  using Value = Data::Value;
  using Size = Data::Size;
  using Allocator = Data::Allocator;

  using value_type = Value;
  using allocator_type = Allocator;
  using size_type = Size;
  using difference_type = std::iter_difference_t<V*>;
  using reference = Value&;
  using const_reference = const Value&;
  using pointer = Value*;
  using const_pointer = const Value*;

  using iterator = Data::iterator;
  using const_iterator = Data::const_iterator;

  using InitPolicy = InitPol;

  constexpr FixedArray() = default;
  explicit constexpr FixedArray(Size size) : allocation_(size) {
    initialize_all();
  }
  constexpr FixedArray(Size size, Allocator&& alloc)
      : allocation_(size, std::forward<Allocator>(alloc)) {
    initialize_all();
  }
  constexpr FixedArray(Size size, const Allocator& alloc) : allocation_(size, alloc) {
    initialize_all();
  }

  template<typename Other>
  explicit constexpr FixedArray(Size size, const Other& value) : allocation_(size) {
    uninit_fill(value);
  }
  template<typename Other>
  constexpr FixedArray(Size size, const Other& value, Allocator&& alloc)
      : allocation_(size, std::forward<Allocator>(alloc)) {
    uninit_fill(value);
  }
  template<typename Other>
  constexpr FixedArray(Size size, const Other& value, const Allocator& alloc)
      : allocation_(size, alloc) {
    uninit_fill(value);
  }

  constexpr FixedArray(std::initializer_list<Value> init) : allocation_(init.size()) {
    std::uninitialized_copy(init.begin(), init.end(), begin());
  }

  // Per-element placement-new construction.
  explicit constexpr FixedArray(UninitializedConstruct /*tag*/, Size size, auto op)
      : allocation_(size) {
    V* ptr = allocation_.data();
    for (Size i = 0; i < size; ++i) {
      op(i, ptr + i);
    }
  }

  constexpr FixedArray(FixedArray&& other) noexcept : allocation_(std::move(other.allocation_)) {}
  // Only valid if the data is fully initialized.
  constexpr FixedArray(const FixedArray& other) : allocation_(other.allocation_.size()) {
    std::uninitialized_copy(other.allocation_.begin(), other.allocation_.end(),
                            allocation_.begin());
  }

  constexpr FixedArray& operator=(FixedArray&& other) noexcept {
    allocation_.destroy_initialized();
    allocation_.move_to_destroyed(std::move(other.allocation_));
    return *this;
  }
  // Only valid if the data is fully initialized.
  constexpr FixedArray& operator=(const FixedArray& other) {
    if (this != &other) {
      allocation_.destroy_initialized();
      allocation_.reallocate_to_destroyed(other.allocation_);
      std::uninitialized_copy(other.allocation_.begin(), other.allocation_.end(),
                              allocation_.begin());
    }
    return *this;
  }

  template<typename... Args>
  void initial_emplace(Size index, Args&&... args)
  requires(std::same_as<InitPol, NoInit>)
  {
    new (this->begin() + index) Value(std::forward<Args>(args)...);
  }
  void initialize(Size index, Value&& value)
  requires(std::same_as<InitPol, NoInit>)
  {
    initial_emplace(index, std::forward<Value>(value));
  }
  void initialize(Size index, const Value& value)
  requires(std::same_as<InitPol, NoInit>)
  {
    initial_emplace(index, value);
  }

  // Only valid if the data is fully initialized.
  ~FixedArray() {
    allocation_.destroy_initialized();
  }

  friend constexpr void swap(FixedArray& lhs, FixedArray& rhs) noexcept {
    using std::swap;
    swap(lhs.allocation_, rhs.allocation_);
  }

  [[nodiscard]] constexpr Size size() const noexcept {
    return allocation_.size();
  }
  [[nodiscard]] constexpr bool empty() const noexcept {
    return allocation_.empty();
  }

  [[nodiscard]] constexpr auto data(this auto&& self) {
    return self.allocation_.data();
  }

  [[nodiscard]] constexpr auto begin(this auto&& self) {
    return self.allocation_.begin();
  }
  [[nodiscard]] constexpr auto end(this auto&& self) {
    return self.allocation_.end();
  }

  [[nodiscard]] constexpr decltype(auto) operator[](this auto&& self, Size index) {
    return self.allocation_[index];
  }

  [[nodiscard]] constexpr decltype(auto) at(this auto&& self, Size index) {
    if (index >= self.size()) {
      throw std::out_of_range{cat("Invalid index: ", index, " >= ", self.size())};
    }
    return self.allocation_[index];
  }

  [[nodiscard]] constexpr decltype(auto) front(this auto&& self) {
    return self.allocation_.front();
  }
  [[nodiscard]] constexpr decltype(auto) back(this auto&& self) {
    return self.allocation_.back();
  }

  [[nodiscard]] constexpr auto span(this auto&& self) {
    return self.allocation_.span();
  }

  [[nodiscard]] friend constexpr bool operator==(const FixedArray& lhs, const FixedArray& rhs) {
    return std::ranges::equal(lhs, rhs);
  }

private:
  constexpr void initialize_all() {
    initialize(allocation_.begin(), allocation_.end());
  }
  static constexpr void initialize(iterator begin, iterator end) {
    InitPolicy::initialize(begin, end);
  }

  template<typename Other>
  constexpr void uninit_fill(const Other& value) {
    std::uninitialized_fill(allocation_.begin(), allocation_.end(), value);
  }

  Data allocation_{};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_ARRAY_FIXED_HPP
