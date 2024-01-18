#ifndef INCLUDE_THESAUROS_CONTAINERS_ARRAY_FIXED_HPP
#define INCLUDE_THESAUROS_CONTAINERS_ARRAY_FIXED_HPP

#include <cstddef>
#include <initializer_list>
#include <memory>
#include <span>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "thesauros/containers/array/initialization-policy.hpp"
#include "thesauros/containers/array/typed-chunk.hpp"

namespace thes {
template<typename TValue, typename TAllocator, typename TInitPolicy>
struct FixedArray {
  using Data = array::TypedChunk<TValue, std::size_t, TAllocator>;

  using Value = Data::Value;
  using Size = Data::Size;
  using Allocator = Data::Allocator;

  using value_type = Value;
  using size_type = Size;

  using iterator = Data::iterator;
  using const_iterator = Data::const_iterator;

  using InitPolicy = TInitPolicy;

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

  template<typename TOther>
  explicit constexpr FixedArray(Size size, const TOther& value) : allocation_(size) {
    uninit_fill(value);
  }
  template<typename TOther>
  constexpr FixedArray(Size size, const TOther& value, Allocator&& alloc)
      : allocation_(size, std::forward<Allocator>(alloc)) {
    uninit_fill(value);
  }
  template<typename TOther>
  constexpr FixedArray(Size size, const TOther& value, const Allocator& alloc)
      : allocation_(size, alloc) {
    uninit_fill(value);
  }

  constexpr FixedArray(std::initializer_list<Value> init) : allocation_(init.size()) {
    std::uninitialized_copy(init.begin(), init.end(), begin());
  }

  constexpr FixedArray(FixedArray&& other) noexcept : allocation_(std::move(other.allocation_)) {}
  // WARNING Only valid if the data is fully initialized!
  constexpr FixedArray(const FixedArray& other) : allocation_(other.allocation_.size()) {
    std::uninitialized_copy(other.allocation_.begin(), other.allocation_.end(),
                            allocation_.begin());
  }

  constexpr FixedArray& operator=(FixedArray&& other) noexcept {
    allocation_.destroy_initialized();
    allocation_.move_to_destroyed(std::move(other.allocation_));
    return *this;
  }
  // WARNING Only valid if the data is fully initialized!
  constexpr FixedArray& operator=(const FixedArray& other) {
    if (this != &other) {
      allocation_.destroy_initialized();
      allocation_.reallocate_to_destroyed(other.allocation_);
      std::uninitialized_copy(other.allocation_.begin(), other.allocation_.end(),
                              allocation_.begin());
    }
    return *this;
  }

  // WARNING Only valid if the data is fully initialized!
  ~FixedArray() {
    allocation_.destroy_initialized();
  }

  [[nodiscard]] constexpr Size size() const noexcept {
    return allocation_.size();
  }
  [[nodiscard]] constexpr bool empty() const noexcept {
    return allocation_.empty();
  }

  [[nodiscard]] constexpr TValue* data() {
    return allocation_.data();
  }
  [[nodiscard]] const constexpr TValue* data() const {
    return allocation_.data();
  }

  [[nodiscard]] constexpr iterator begin() noexcept {
    return allocation_.begin();
  }
  [[nodiscard]] constexpr const_iterator begin() const noexcept {
    return allocation_.begin();
  }
  [[nodiscard]] constexpr iterator end() noexcept {
    return allocation_.end();
  }
  [[nodiscard]] constexpr const_iterator end() const noexcept {
    return allocation_.end();
  }

  [[nodiscard]] constexpr Value& operator[](Size index) {
    return allocation_[index];
  }
  [[nodiscard]] constexpr const Value& operator[](Size index) const {
    return allocation_[index];
  }

  [[nodiscard]] constexpr Value& at(Size index) {
    if (index >= size()) {
      throw std::out_of_range(
        (std::stringstream{} << "Invalid index: " << index << ' ' << size()).str());
    }
    return allocation_[index];
  }
  [[nodiscard]] constexpr const Value& at(Size index) const {
    if (index >= size()) {
      throw std::out_of_range(
        (std::stringstream{} << "Invalid index: " << index << ' ' << size()).str());
    }
    return allocation_[index];
  }

  [[nodiscard]] constexpr Value& front() {
    return allocation_.front();
  }
  [[nodiscard]] constexpr const Value& front() const {
    return allocation_.front();
  }

  [[nodiscard]] constexpr Value& back() {
    return allocation_.back();
  }
  [[nodiscard]] constexpr const Value& back() const {
    return allocation_.back();
  }

  [[nodiscard]] constexpr std::span<Value> span() {
    return {begin(), end()};
  }
  [[nodiscard]] constexpr std::span<const Value> span() const {
    return {begin(), end()};
  }

private:
  constexpr void initialize_all() {
    initialize(allocation_.begin(), allocation_.end());
  }
  static constexpr void initialize(iterator begin, iterator end) {
    InitPolicy::initialize(begin, end);
  }

  template<typename TOther>
  constexpr void uninit_fill(const TOther& value) {
    std::uninitialized_fill(allocation_.begin(), allocation_.end(), value);
  }

  Data allocation_{};
};

template<typename TValue, typename TAlloc = std::allocator<TValue>>
struct FixedArrayValue : public FixedArray<TValue, TAlloc, array::ValueInitPolicy> {
  using Parent = FixedArray<TValue, TAlloc, array::ValueInitPolicy>;

  using Parent::Parent;
};

template<typename TValue, typename TAlloc = std::allocator<TValue>>
struct FixedArrayDefault : public FixedArray<TValue, TAlloc, array::DefaultInitPolicy> {
  using Parent = FixedArray<TValue, TAlloc, array::DefaultInitPolicy>;

  using Parent::Parent;
};

template<typename TValue, typename TAlloc = std::allocator<TValue>>
struct FixedArrayUninit : public FixedArray<TValue, TAlloc, array::NoInitPolicy> {
  using Parent = FixedArray<TValue, TAlloc, array::NoInitPolicy>;
  using Size = Parent::Size;
  using Value = Parent::Value;

  using Parent::Parent;

  template<typename... TArgs>
  void initial_emplace(Size index, TArgs&&... args) {
    new (this->begin() + index) Value(std::forward<TArgs>(args)...);
  }
  void initialize(Size index, Value&& value) {
    initial_emplace(index, std::forward<Value>(value));
  }
  void initialize(Size index, const Value& value) {
    initial_emplace(index, value);
  }
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_ARRAY_FIXED_HPP
