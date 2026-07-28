// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_ARRAY_TYPED_CHUNK_HPP
#define INCLUDE_THESAUROS_CONTAINERS_ARRAY_TYPED_CHUNK_HPP

#include <cassert>
#include <memory>
#include <span>
#include <type_traits>
#include <utility>


namespace thes::array {
/**
 * A helper class managing a typed chunk of memory.
 *
 * Managing the lifetime of elements is the responsibility of the user; elements are neither
 * constructed nor destroyed by this class.
 */
template<typename V, typename S, typename Alloc>
struct TypedChunk {
  using Value = V;
  using Size = S;
  using Allocator = Alloc;

  using value_type = Value;
  using allocator_type = Allocator;
  using size_type = Size;
  using difference_type = std::make_signed_t<Size>;
  using reference = Value&;
  using const_reference = const Value&;
  using pointer = Value*;
  using const_pointer = const Value*;
  using iterator = pointer;
  using const_iterator = const_pointer;

  constexpr TypedChunk() = default;
  explicit constexpr TypedChunk(const Allocator& alloc) : alloc_(alloc) {}
  explicit constexpr TypedChunk(Allocator&& alloc) : alloc_(std::forward<Allocator>(alloc)) {}

  explicit constexpr TypedChunk(Size size) : begin_(allocate_memory(size)), end_(begin_ + size) {}
  constexpr TypedChunk(Size size, Allocator&& alloc)
      : alloc_(std::forward<Allocator>(alloc)), begin_(allocate_memory(size)), end_(begin_ + size) {
  }
  constexpr TypedChunk(Size size, const Allocator& alloc)
      : alloc_(alloc), begin_(allocate_memory(size)), end_(begin_ + size) {}

  constexpr TypedChunk(TypedChunk&& other) noexcept
      : alloc_(std::move(other.alloc_)), begin_(other.begin_), end_(other.end_) {
    other.begin_ = nullptr;
    other.end_ = nullptr;
  }
  // Copying requires knowledge of the elements’ initialization state, so it is disabled.
  TypedChunk(const TypedChunk&) = delete;
  // Assignment should use the explicit functions defined below.
  TypedChunk& operator=(TypedChunk&&) = delete;
  TypedChunk& operator=(const TypedChunk&) = delete;

  // Only deallocates; destruction of the elements has to be handled by the deriving class.
  constexpr ~TypedChunk() {
    deallocate();
  }

  friend constexpr void swap(TypedChunk& v1, TypedChunk& v2) noexcept {
    using std::swap;
    swap(v1.alloc_, v2.alloc_);
    swap(v1.begin_, v2.begin_);
    swap(v1.end_, v2.end_);
  }

  [[nodiscard]] constexpr auto data(this auto&& self) {
    return const_or_mut<decltype(self)>(self.begin_);
  }
  /** Allows users to skirt const-correctness */
  [[nodiscard]] constexpr Value* mutable_data(this auto&& self) {
    return self.begin_;
  }

  [[nodiscard]] constexpr Size size() const {
    return static_cast<Size>(end_ - begin_);
  }
  [[nodiscard]] constexpr bool empty() const {
    return begin_ == end_;
  }

  [[nodiscard]] constexpr auto begin(this auto&& self) {
    return const_or_mut<decltype(self)>(self.begin_);
  }
  [[nodiscard]] constexpr auto end(this auto&& self) {
    return const_or_mut<decltype(self)>(self.end_);
  }

  [[nodiscard]] constexpr decltype(auto) operator[](this auto&& self, Size index) {
    assert(index < self.size());
    return *(const_or_mut<decltype(self)>(self.begin_) + index);
  }
  [[nodiscard]] constexpr decltype(auto) front(this auto&& self) {
    assert(!self.empty());
    return *const_or_mut<decltype(self)>(self.begin_);
  }
  [[nodiscard]] constexpr decltype(auto) back(this auto&& self) {
    assert(!self.empty());
    return *(const_or_mut<decltype(self)>(self.end_) - 1);
  }

  [[nodiscard]] constexpr Value* allocate_memory(const Size size) {
    return std::allocator_traits<Allocator>::allocate(alloc_, size);
  }
  void allocate(const Size size) {
    assert(begin_ == nullptr && end_ == nullptr);
    begin_ = allocate_memory(size);
    end_ = begin_ + size;
  }

  const Allocator& allocator() const {
    return alloc_;
  }

  // Only valid if the data is fully initialized.
  constexpr void destroy_initialized() {
    std::destroy(begin_, end_);
  }
  constexpr void deallocate() {
    if (begin_ != nullptr) {
      assert(end_ != nullptr);
      std::allocator_traits<Allocator>::deallocate(alloc_, begin_,
                                                   static_cast<Size>(end_ - begin_));
      begin_ = nullptr;
      end_ = nullptr;
    } else {
      assert(end_ == nullptr);
    }
  }

  void copy_allocator(const Allocator& allocator) {
    alloc_ = allocator;
  }

  // Requires the data to be destroyed already.
  void move_to_destroyed(TypedChunk&& other) noexcept {
    deallocate();

    alloc_ = std::move(other.alloc_);
    begin_ = other.begin_;
    other.begin_ = nullptr;

    end_ = other.end_;
    other.end_ = nullptr;
  }
  // Requires the data to be destroyed already.
  void reallocate_to_destroyed(const TypedChunk& other) {
    assert(this != &other);

    deallocate();

    alloc_ = other.alloc_;
    const auto size = static_cast<Size>(other.end_ - other.begin_);
    begin_ = allocate_memory(size);
    end_ = begin_ + size;
  }

  constexpr void allocate_to_empty(Size size) {
    assert(begin_ == nullptr && end_ == nullptr);
    begin_ = allocate_memory(size);
    end_ = begin_ + size;
  }
  // Only valid if the data is fully initialized.
  constexpr void expand(Size new_size, auto&& mover) {
    assert(new_size > size());

    Value* new_begin = allocate_memory(new_size);
    mover(begin_, end_, new_begin);
    deallocate();

    begin_ = new_begin;
    end_ = new_begin + new_size;
  }
  constexpr void expand(Size new_size, iterator data_end) {
    expand(new_size, [data_end](iterator old_begin, iterator /*old_end*/, iterator new_begin) {
      std::uninitialized_move(old_begin, data_end, new_begin);
      std::destroy(old_begin, data_end);
    });
  }
  // Only valid if the data is fully initialized.
  constexpr void shrink(Size new_size) {
    assert(new_size < size());

    Value* new_begin = allocate_memory(new_size);
    std::uninitialized_move(begin_, begin_ + new_size, new_begin);

    destroy_initialized();
    deallocate();

    begin_ = new_begin;
    end_ = new_begin + new_size;
  }

  [[nodiscard]] constexpr auto span(this auto&& self) {
    return std::span{
      const_or_mut<decltype(self)>(self.begin_),
      const_or_mut<decltype(self)>(self.end_),
    };
  }

private:
  /** Converts `p` to a `const TValue*` if `Self` is const, otherwise leaves it as `TValue*`. */
  template<typename Self>
  static constexpr auto const_or_mut(Value* p) noexcept {
    if constexpr (std::is_const_v<std::remove_reference_t<Self>>) {
      return static_cast<const Value*>(p);
    } else {
      return p;
    }
  }

  [[no_unique_address]] Alloc alloc_{};
  V* begin_{nullptr};
  V* end_{nullptr};
};
} // namespace thes::array

#endif // INCLUDE_THESAUROS_CONTAINERS_ARRAY_TYPED_CHUNK_HPP
