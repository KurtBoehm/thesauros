#ifndef INCLUDE_THESAUROS_CONTAINERS_ARRAY_FIXED_ALLOC_HPP
#define INCLUDE_THESAUROS_CONTAINERS_ARRAY_FIXED_ALLOC_HPP

#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <utility>

#include "thesauros/containers/array/typed-chunk.hpp"

namespace thes {
template<typename TValue, typename TAllocator = std::allocator<TValue>>
struct FixedAllocArray {
  using Data = array::TypedChunk<TValue, std::size_t, TAllocator>;

  using Value = Data::Value;
  using Size = Data::Size;
  using Allocator = Data::Allocator;

  using value_type = Value;
  using size_type = Size;

  using iterator = Data::iterator;
  using const_iterator = Data::const_iterator;

  constexpr FixedAllocArray() = default;
  explicit constexpr FixedAllocArray(const Allocator& alloc) : allocation_(alloc) {}
  explicit constexpr FixedAllocArray(Allocator&& alloc)
      : allocation_(std::forward<Allocator>(alloc)) {}

  constexpr FixedAllocArray(std::initializer_list<Value> init) : allocation_(init.size()) {
    std::uninitialized_copy(init.begin(), init.end(), begin());
  }

  constexpr FixedAllocArray(FixedAllocArray&& other) noexcept
      : allocation_(std::move(other.allocation_)), data_end_(other.data_end_) {
    other.data_end_ = nullptr;
  }
  // WARNING Only valid if the data is fully initialized!
  constexpr FixedAllocArray(const FixedAllocArray& other)
      : allocation_(other.allocation_.size(), other.allocation_),
        data_end_(allocation_.begin() + other.size()) {
    std::uninitialized_copy(other.allocation_.begin(), other.allocation_.end(),
                            allocation_.begin());
  }

  constexpr FixedAllocArray& operator=(FixedAllocArray&& other) noexcept {
    destroy();
    allocation_.move_to_destroyed(std::move(other.allocation_));
    data_end_ = other.data_end_;
    other.data_end_ = nullptr;
    return *this;
  }
  // WARNING Only valid if the data is fully initialized!
  constexpr FixedAllocArray& operator=(const FixedAllocArray& other) {
    if (this != &other) {
      destroy();
      allocation_.reallocate_to_destroyed(other.allocation_);
      const_iterator other_begin = other.allocation_.begin();
      const_iterator other_data_end = other.data_end_;
      std::uninitialized_copy(other_begin, other_data_end, allocation_.begin());
      data_end_ = allocation_.begin() + other.size();
    }
    return *this;
  }

  static constexpr FixedAllocArray create_with_capacity(std::size_t capacity) {
    return FixedAllocArray(std::in_place_type<Data>, capacity);
  }

  ~FixedAllocArray() {
    destroy();
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

  [[nodiscard]] constexpr TValue* data() {
    return allocation_.data();
  }
  [[nodiscard]] constexpr const TValue* data() const {
    return allocation_.data();
  }

  [[nodiscard]] constexpr iterator begin() noexcept {
    return allocation_.begin();
  }
  [[nodiscard]] constexpr const_iterator begin() const noexcept {
    return allocation_.begin();
  }
  [[nodiscard]] constexpr iterator end() noexcept {
    return data_end_;
  }
  [[nodiscard]] constexpr const_iterator end() const noexcept {
    return data_end_;
  }

  [[nodiscard]] constexpr Value& operator[](Size index) {
    assert(index < size());
    return allocation_[index];
  }
  [[nodiscard]] constexpr const Value& operator[](Size index) const {
    assert(index < size());
    return allocation_[index];
  }

  [[nodiscard]] constexpr Value& front() {
    assert(!empty());
    return allocation_.front();
  }
  [[nodiscard]] constexpr const Value& front() const {
    assert(!empty());
    return allocation_.front();
  }

  [[nodiscard]] constexpr Value& back() {
    assert(!empty());
    return *(data_end_ - 1);
  }
  [[nodiscard]] constexpr const Value& back() const {
    assert(!empty());
    return *(data_end_ - 1);
  }

  constexpr void reserve(Size new_alloc) {
    if (new_alloc > allocation_.size()) {
      const Size current_size = size();
      allocation_.expand(new_alloc, data_end_);
      data_end_ = allocation_.begin() + current_size;
    }
  }
  template<typename... TArgs>
  void emplace_back(TArgs&&... args) {
    assert(data_end_ != allocation_.end());

    new (data_end_) TValue(std::forward<TArgs>(args)...);
    ++data_end_;
  }
  void push_back(TValue&& value) {
    emplace_back(std::forward<TValue>(value));
  }
  void push_back(const TValue& value) {
    emplace_back(TValue(value));
  }

  constexpr void clear() {
    destroy();
    data_end_ = allocation_.begin();
  }

private:
  // WARNING Only valid if the data is fully initialized!
  constexpr void destroy() {
    std::destroy(allocation_.begin(), data_end_);
  }

  template<typename... TArgs>
  explicit FixedAllocArray(std::in_place_type_t<Data> /*tag*/, TArgs&&... args)
      : allocation_(std::forward<TArgs>(args)...) {}

  Data allocation_{};
  Value* data_end_{allocation_.begin()};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_ARRAY_FIXED_ALLOC_HPP
