#ifndef INCLUDE_THESAUROS_CONTAINERS_ARRAY_TYPED_CHUNK_HPP
#define INCLUDE_THESAUROS_CONTAINERS_ARRAY_TYPED_CHUNK_HPP

#include <cassert>
#include <memory>
#include <span>
#include <utility>

#include "thesauros/io.hpp"

namespace thes::array {
// A helper class managing a typed chunk of memory.
//
// Note that managing the lifetime of elements is the responsibility of the user — elements are
// neither constructed or destroyed!
template<typename TValue, typename TSize, typename TAllocator>
struct TypedChunk {
  using Value = TValue;
  using Size = TSize;
  using Allocator = TAllocator;

  using value_type = Value;
  using iterator = TValue*;
  using const_iterator = const TValue*;

  static TypedChunk from_file(FileReader& reader) {
    TypedChunk chunk(reader.read(type_tag<Size>));
    reader.read(chunk.span());
    return chunk;
  }

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
  // As this needs to copy data of unknown initialization status, it is disabled
  TypedChunk(const TypedChunk&) = delete;
  // Assignments should use the explicit functions defined below
  TypedChunk& operator=(TypedChunk&&) = delete;
  TypedChunk& operator=(const TypedChunk&) = delete;

  // WARNING Only deallocates — destruction of the elements has to be handled by the deriving class!
  constexpr ~TypedChunk() {
    deallocate();
  }

  friend void swap(TypedChunk& v1, TypedChunk& v2) noexcept {
    using std::swap;
    swap(v1.alloc_, v2.alloc_);
    swap(v1.begin_, v2.begin_);
    swap(v1.end_, v2.end_);
  }

  [[nodiscard]] constexpr TValue* data() {
    return begin_;
  }
  [[nodiscard]] constexpr const TValue* data() const {
    return begin_;
  }

  [[nodiscard]] constexpr Size size() const {
    return static_cast<Size>(end_ - begin_);
  }
  [[nodiscard]] constexpr bool empty() const {
    return begin_ == end_;
  }

  [[nodiscard]] constexpr iterator begin() {
    return begin_;
  }
  [[nodiscard]] constexpr const_iterator begin() const {
    return begin_;
  }

  [[nodiscard]] constexpr iterator end() {
    return end_;
  }
  [[nodiscard]] constexpr const_iterator end() const {
    return end_;
  }

  [[nodiscard]] constexpr Value& operator[](Size index) {
    assert(index < size());
    return begin_[index];
  }
  [[nodiscard]] constexpr const Value& operator[](Size index) const {
    assert(index < size());
    return begin_[index];
  }

  [[nodiscard]] constexpr Value& front() {
    assert(!empty());
    return *begin_;
  }
  [[nodiscard]] constexpr const Value& front() const {
    assert(!empty());
    return *begin_;
  }

  [[nodiscard]] constexpr Value& back() {
    assert(!empty());
    return *(end_ - 1);
  }
  [[nodiscard]] constexpr const Value& back() const {
    assert(!empty());
    return *(end_ - 1);
  }

  [[nodiscard]] constexpr TValue* allocate_memory(const Size size) {
    return std::allocator_traits<Allocator>::allocate(alloc_, size);
  }
  void allocate(const Size size) {
    assert(begin_ == nullptr && end_ == nullptr);
    begin_ = allocate_memory(size);
    end_ = begin_ + size;
  }

  // WARNING Only valid if the data is fully initialized!
  constexpr void destroy_initialized() {
    std::destroy(begin_, end_);
  }
  constexpr void deallocate() {
    if (begin_ != nullptr) {
      assert(end_ != nullptr);
      std::allocator_traits<Allocator>::deallocate(alloc_, begin_,
                                                   static_cast<Size>(end_ - begin_));
    } else {
      assert(end_ == nullptr);
    }
  }

  void copy_allocator(const Allocator& allocator) {
    alloc_ = allocator;
  }

  // WARNING Requires the data to be destroyed already!
  void move_to_destroyed(TypedChunk&& other) noexcept {
    deallocate();

    alloc_ = std::move(other.alloc_);
    begin_ = other.begin_;
    other.begin_ = nullptr;

    end_ = other.end_;
    other.end_ = nullptr;
  }
  // WARNING Requires the data to be destroyed already!
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
  // WARNING Only valid if the data is fully initialized!
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
  // WARNING Only valid if the data is fully initialized!
  constexpr void shrink(Size new_size) {
    assert(new_size < size());

    Value* new_begin = allocate_memory(new_size);
    std::uninitialized_move(begin_, begin_ + new_size, new_begin);

    destroy_initialized();
    deallocate();

    begin_ = new_begin;
    end_ = new_begin + new_size;
  }

  std::span<const Value> span() const {
    return std::span{begin_, end_};
  }
  std::span<Value> span() {
    return std::span{begin_, end_};
  }

  void to_file(FileWriter& writer) const {
    const Size stored_size = size();
    writer.write(std::span{&stored_size, 1});
    writer.write(span());
  }

private:
  [[no_unique_address]] TAllocator alloc_{};
  TValue* begin_{nullptr};
  TValue* end_{nullptr};
};
} // namespace thes::array

#endif // INCLUDE_THESAUROS_CONTAINERS_ARRAY_TYPED_CHUNK_HPP
