// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_ARRAY_FIXED_ALLOC_HPP
#define INCLUDE_THESAUROS_CONTAINERS_ARRAY_FIXED_ALLOC_HPP

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

#include "thesauros/containers/array/typed-chunk.hpp"

namespace thes {
template<typename V, typename Alloc = std::allocator<V>>
struct FixedAllocArray {
  using Data = TypedChunk<V, std::size_t, Alloc>;

  using Value = Data::Value;
  using Size = Data::Size;
  using Allocator = Data::Allocator;

  using value_type = Value;
  using allocator_type = Allocator;
  using size_type = Size;
  using difference_type = std::iter_difference_t<Value*>;
  using reference = Value&;
  using const_reference = const Value&;
  using pointer = Value*;
  using const_pointer = const Value*;

  using iterator = Data::iterator;
  using const_iterator = Data::const_iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  constexpr FixedAllocArray() = delete;
  explicit constexpr FixedAllocArray(const Allocator& alloc) : allocation_(alloc) {}
  explicit constexpr FixedAllocArray(Allocator&& alloc)
      : allocation_(std::forward<Allocator>(alloc)) {}

  constexpr FixedAllocArray(std::initializer_list<Value> init)
      : allocation_(init.size()), data_end_(allocation_.data() + init.size()) {
    std::uninitialized_copy(init.begin(), init.end(), begin());
  }

  constexpr FixedAllocArray(FixedAllocArray&& other) noexcept
      : allocation_(std::move(other.allocation_)), data_end_(other.data_end_) {
    other.data_end_ = nullptr;
  }
  constexpr FixedAllocArray(const FixedAllocArray& other)
      : allocation_(other.allocation_.size(), other.allocation_.allocator()),
        data_end_(allocation_.data() + other.size()) {
    std::uninitialized_copy(other.allocation_.begin(), other.allocation_.end(),
                            allocation_.begin());
  }

  FixedAllocArray& operator=(const FixedAllocArray&) = delete;
  FixedAllocArray& operator=(FixedAllocArray&&) = delete;

  static constexpr FixedAllocArray create_with_capacity(std::size_t capacity) {
    return FixedAllocArray(std::in_place_type<Data>, capacity);
  }

  ~FixedAllocArray() {
    destroy();
  }

  friend constexpr void swap(FixedAllocArray& lhs, FixedAllocArray& rhs) noexcept {
    using std::swap;
    swap(lhs.allocation_, rhs.allocation_);
    swap(lhs.data_end_, rhs.data_end_);
  }

  [[nodiscard]] constexpr Size size() const noexcept {
    return static_cast<Size>(data_end_ - allocation_.begin());
  }
  [[nodiscard]] constexpr Size allocation_size() const noexcept {
    return allocation_.size();
  }
  [[nodiscard]] constexpr bool empty() const noexcept {
    return allocation_.begin() == data_end_;
  }

  [[nodiscard]] constexpr auto data(this auto&& self) {
    return self.allocation_.data();
  }

  [[nodiscard]] constexpr auto begin(this auto&& self) noexcept {
    return self.allocation_.begin();
  }
  template<typename Self>
  [[nodiscard]] constexpr std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>,
                                             const_iterator, iterator>
  end(this Self&& self) noexcept {
    return self.data_end_;
  }

  [[nodiscard]] constexpr auto rbegin(this auto&& self) noexcept {
    return std::reverse_iterator{self.end()};
  }
  [[nodiscard]] constexpr auto rend(this auto&& self) noexcept {
    return std::reverse_iterator{self.begin()};
  }

  [[nodiscard]] constexpr decltype(auto) operator[](this auto&& self, Size index) {
    assert(index < self.size());
    return self.allocation_[index];
  }

  [[nodiscard]] constexpr decltype(auto) front(this auto&& self) {
    assert(!self.empty());
    return self.allocation_.front();
  }
  template<typename Self>
  [[nodiscard]] constexpr std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>,
                                             const_reference, reference>
  back(this Self&& self) {
    assert(!self.empty());
    return *(self.data_end_ - 1);
  }

  template<typename... Args>
  requires(std::constructible_from<Value, Args...>)
  Value& emplace_back(Args&&... args) {
    assert(data_end_ != allocation_.end());
    new (data_end_) Value(std::forward<Args>(args)...);
    ++data_end_;
    return *(data_end_ - 1);
  }
  Value& push_back(Value&& value) {
    return emplace_back(std::forward<Value>(value));
  }
  Value& push_back(const Value& value) {
    return emplace_back(value);
  }

  void pop_back() {
    assert(!empty());
    std::destroy_at(--data_end_);
  }

  constexpr void clear() {
    destroy();
    data_end_ = allocation_.begin();
  }

  [[nodiscard]] friend constexpr bool operator==(const FixedAllocArray& lhs,
                                                 const FixedAllocArray& rhs) {
    return std::ranges::equal(lhs, rhs);
  }

private:
  constexpr void destroy() {
    std::destroy(allocation_.begin(), data_end_);
  }

  template<typename... Args>
  explicit FixedAllocArray(std::in_place_type_t<Data> /*tag*/, Args&&... args)
      : allocation_(std::forward<Args>(args)...) {}

  Data allocation_{};
  Value* data_end_{allocation_.begin()};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_ARRAY_FIXED_ALLOC_HPP
