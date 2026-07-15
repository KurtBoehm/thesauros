// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_ARRAY_DYNAMIC_HPP
#define INCLUDE_THESAUROS_CONTAINERS_ARRAY_DYNAMIC_HPP

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

#include "thesauros/containers/array/growth-policy.hpp"
#include "thesauros/containers/array/initialization-policy.hpp"
#include "thesauros/containers/array/typed-chunk.hpp"

namespace thes {
/**
 * A generic dynamic array class.
 *
 * The initialization policy `IP` is used to initialize newly created elements when no constructor
 * parameters are given, e.g. in the fixed-size constructor and `resize`.
 *
 * Independently of the initialization policy, many functions (e.g. copy construction and
 * assignment, destruction, `resize`, `push_back`, etc.) assume that all elements are in a state
 * that permits copying or moving (depending on the operation); the elements need to have been
 * constructed if they are of a class type. If an initialization policy that does not initialize
 * its elements (e.g. `NoInit`) is used, it is the responsibility of the user to ensure that this
 * assumption holds.
 *
 * In any case, no helper function for initialization is provided, as such a function makes no
 * sense if the elements are implicitly initialized by the initialization policy.
 *
 * The growth policy `GP` determines the growth behavior when e.g. `resize` or `push_back` are
 * called.
 */
template<typename TValue, typename TInitPolicy = DefaultInit,
         typename TGrowthPolicy = DoublingGrowth, typename TAllocator = std::allocator<TValue>>
struct DynamicArray {
  using Data = array::TypedChunk<TValue, std::size_t, TAllocator>;

  using Value = Data::Value;
  using Size = Data::Size;
  using Allocator = Data::Allocator;

  using value_type = Value;
  using allocator_type = Allocator;
  using size_type = Size;
  using difference_type = std::iter_difference_t<TValue*>;
  using reference = Value&;
  using const_reference = const Value&;
  using pointer = Value*;
  using const_pointer = const Value*;

  using iterator = Data::iterator;
  using const_iterator = Data::const_iterator;

  using InitPolicy = TInitPolicy;
  using GrowthPolicy = TGrowthPolicy;

  constexpr DynamicArray() = default;
  explicit constexpr DynamicArray(const Allocator& alloc) : allocation_(alloc) {}
  explicit constexpr DynamicArray(Allocator&& alloc)
      : allocation_(std::forward<Allocator>(alloc)) {}

  explicit constexpr DynamicArray(Size size) : allocation_(size) {
    initialize_all();
  }
  constexpr DynamicArray(Size size, Allocator&& alloc)
      : allocation_(size, std::forward<Allocator>(alloc)) {
    initialize_all();
  }
  constexpr DynamicArray(Size size, const Allocator& alloc) : allocation_(size, alloc) {
    initialize_all();
  }

  explicit constexpr DynamicArray(Size size, const TValue& value) : allocation_(size) {
    std::uninitialized_fill(begin(), end(), value);
  }

  constexpr DynamicArray(std::initializer_list<Value> init) : allocation_(init.size()) {
    std::uninitialized_copy(init.begin(), init.end(), begin());
  }

  constexpr DynamicArray(DynamicArray&& other) noexcept
      : allocation_(std::move(other.allocation_)), data_end_(other.data_end_) {
    other.data_end_ = nullptr;
  }
  // Only valid if the data is fully initialized.
  constexpr DynamicArray(const DynamicArray& other)
      : allocation_(other.allocation_.size(), other.allocation_.allocator()),
        data_end_(allocation_.begin() + other.size()) {
    const Value* other_begin = other.allocation_.begin();
    const Value* other_end = other.data_end_;
    std::uninitialized_copy(other_begin, other_end, allocation_.begin());
  }

  constexpr DynamicArray& operator=(DynamicArray&& other) noexcept {
    destroy();
    allocation_.move_to_destroyed(std::move(other.allocation_));
    data_end_ = other.data_end_;
    other.data_end_ = nullptr;
    return *this;
  }
  // Only valid if the data is fully initialized.
  constexpr DynamicArray& operator=(const DynamicArray& other) {
    if (this != &other) {
      destroy();
      allocation_.reallocate_to_destroyed(other.allocation_);
      std::uninitialized_copy(other.allocation_.begin(), other.end(), allocation_.begin());
      data_end_ = allocation_.begin() + other.size();
    }
    return *this;
  }

  // Only valid if the data is fully initialized.
  constexpr ~DynamicArray() {
    destroy();
  }

  friend constexpr void swap(DynamicArray& lhs, DynamicArray& rhs) noexcept {
    using std::swap;
    swap(lhs.allocation_, rhs.allocation_);
    swap(lhs.data_end_, rhs.data_end_);
  }

  template<typename... TArgs>
  void initial_emplace(Size index, TArgs&&... args)
  requires(std::same_as<TInitPolicy, NoInit>)
  {
    new (this->begin() + index) Value(std::forward<TArgs>(args)...);
  }
  void initialize(Size index, Value&& value)
  requires(std::same_as<TInitPolicy, NoInit>)
  {
    initial_emplace(index, std::forward<Value>(value));
  }
  void initialize(Size index, const Value& value)
  requires(std::same_as<TInitPolicy, NoInit>)
  {
    initial_emplace(index, value);
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

  [[nodiscard]] friend constexpr bool operator==(const DynamicArray& lhs, const DynamicArray& rhs) {
    return std::ranges::equal(lhs, rhs);
  }

  constexpr void clear() {
    destroy();
    data_end_ = allocation_.begin();
  }
  constexpr void clear_memory() {
    destroy();
    allocation_.deallocate();
    data_end_ = nullptr;
  }

  constexpr void expand(Size new_size) {
    assert(new_size > size());
    if (new_size <= allocation_.size()) {
      iterator new_data_end = allocation_.begin() + new_size;
      initialize(data_end_, new_data_end);
      data_end_ = new_data_end;
    } else {
      const Size old_size = size();
      allocation_expand(new_size, [old_size, new_size](iterator new_begin) {
        initialize(new_begin + old_size, new_begin + new_size);
      });
      data_end_ = allocation_.begin() + new_size;
    }
  }
  constexpr void shrink(Size new_size) {
    assert(new_size < size());

    iterator new_data_end = allocation_.begin() + new_size;
    std::destroy(new_data_end, data_end_);
    data_end_ = new_data_end;
  }
  constexpr void resize(Size new_size) {
    if (new_size < size()) {
      shrink(new_size);
    }
    if (new_size > size()) {
      expand(new_size);
    }
  }

  constexpr void reserve(Size new_alloc) {
    if (new_alloc > allocation_.size()) {
      const Size current_size = size();
      allocation_expand(new_alloc, [](iterator /*new_begin*/) {});
      data_end_ = allocation_.begin() + current_size;
    }
  }

  template<typename... TArgs>
  constexpr Value& emplace_back(TArgs&&... args) {
    const Size old_size = size();
    const Size new_size = old_size + 1;
    if (new_size <= allocation_.size()) {
      new (data_end_) Value(std::forward<TArgs>(args)...);
      ++data_end_;
    } else {
      allocation_expand(new_size, [old_size, &args...](iterator new_begin) {
        new (new_begin + old_size) Value(std::forward<TArgs>(args)...);
      });
      data_end_ = allocation_.begin() + new_size;
    }
    return *(data_end_ - 1);
  }
  constexpr Value& push_back(Value&& value) {
    return emplace_back(std::forward<Value>(value));
  }
  constexpr Value& push_back(const Value& value) {
    return emplace_back(value);
  }
  constexpr void pop_back() {
    assert(!empty());
    std::destroy_at(--data_end_);
  }

  constexpr iterator erase(iterator pos) {
    assert(pos != data_end_);
    std::move(pos + 1, data_end_, pos);
    std::destroy_at(data_end_);
    --data_end_;
    return pos;
  }
  constexpr iterator erase(iterator first, iterator last) {
    assert(first != data_end_);
    iterator new_end = std::move(last, data_end_, first);
    std::destroy(new_end, data_end_);
    data_end_ = new_end;
    return last;
  }

  constexpr iterator insert_any(const_iterator pos, Size ins_size, Size pad_end = 0) {
    const auto offset = pos - allocation_.begin();
    iterator mut_pos = allocation_.begin() + offset;

    const Size old_size = size();
    const Size add_size = ins_size + pad_end;
    const Size new_size = old_size + add_size;

    if (new_size <= allocation_.size()) {
      std::move_backward(mut_pos, data_end_, data_end_ + ins_size);
      data_end_ += add_size;
      const Size soff = *safe_cast<Size>(offset);
      const Size post_move = soff + ins_size;
      const Size moved_end = std::min(post_move, old_size);
      initialize(allocation_.begin() + moved_end, allocation_.begin() + post_move);
      initialize(data_end_ - pad_end, data_end_);
      return mut_pos;
    }

    allocation_.expand(grown_size(new_size),
                       [&](iterator old_begin, iterator old_end, iterator new_begin) {
                         iterator moved_start = new_begin + offset;
                         iterator moved_end = moved_start + ins_size;

                         std::uninitialized_move(old_begin, mut_pos, new_begin);
                         initialize(moved_start, moved_end);
                         std::uninitialized_move(mut_pos, old_end, moved_end);
                         initialize(new_begin + (old_size + ins_size), new_begin + new_size);

                         std::destroy(old_begin, data_end_);
                       });
    data_end_ = allocation_.begin() + new_size;
    return allocation_.begin() + offset;
  }

  constexpr iterator insert(iterator pos, Value value) {
    const auto offset = pos - allocation_.begin();
    iterator mut_pos = allocation_.begin() + offset;

    const Size old_size = size();
    const Size new_size = old_size + 1;

    if (new_size <= allocation_.size()) {
      std::move_backward(mut_pos, data_end_, data_end_ + 1);
      ++data_end_;
      *mut_pos = std::move(value);
      return mut_pos;
    }

    allocation_.expand(grown_size(new_size),
                       [&](iterator old_begin, iterator /*old_end*/, iterator new_begin) {
                         iterator target = new_begin + offset;

                         std::uninitialized_move(old_begin, mut_pos, new_begin);
                         std::uninitialized_move(mut_pos, data_end_, target + 1);

                         std::construct_at(target, std::move(value));

                         std::destroy(old_begin, data_end_);
                       });
    data_end_ = allocation_.begin() + new_size;
    return allocation_.begin() + offset;
  }

private:
  constexpr void allocation_expand(const Size new_size, auto&& initializer) {
    allocation_.expand(
      grown_size(new_size),
      [&, old_data_end = data_end_](iterator old_begin, iterator /*old_end*/, iterator new_begin) {
        std::uninitialized_move(old_begin, old_data_end, new_begin);
        initializer(new_begin);
        std::destroy(old_begin, old_data_end);
      });
  }

  constexpr void initialize_all() {
    initialize(allocation_.begin(), data_end_);
  }
  static constexpr void initialize(iterator begin, iterator end) {
    InitPolicy::initialize(begin, end);
  }

  constexpr Size grown_size(Size new_size_lower_bound) const {
    return GrowthPolicy::new_allocation_size(size(), new_size_lower_bound);
  }

  // Only valid if the data is fully initialized.
  constexpr void destroy() {
    std::destroy(allocation_.begin(), data_end_);
  }

  [[nodiscard]] constexpr bool is_allocation_full() const {
    return allocation_.end() == data_end_;
  }

  Data allocation_{};
  Value* data_end_{allocation_.end()};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_ARRAY_DYNAMIC_HPP
