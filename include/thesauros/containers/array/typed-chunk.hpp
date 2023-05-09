#ifndef INCLUDE_THESAUROS_CONTAINERS_ARRAY_TYPED_CHUNK_HPP
#define INCLUDE_THESAUROS_CONTAINERS_ARRAY_TYPED_CHUNK_HPP

#include <memory>
#include <utility>

namespace thes::array {
/**
 * @brief A helper class managing a typed chunk of memory.
 *
 *  Note that managing the lifetime of elements is the responsibility of the user — elements are
 *  neither constructed or destroyed!
 *
 * @tparam T The type of the elements.
 * @tparam S The type used for sizes and indices.
 * @tparam A The allocator
 */
template<typename TValue, typename TSize, typename TAllocator>
struct TypedChunk : public TAllocator {
  using Value = TValue;
  using Size = TSize;
  using Allocator = TAllocator;

  using iterator = TValue*;
  using const_iterator = const TValue*;

  constexpr TypedChunk() = default;
  constexpr explicit TypedChunk(const Allocator& alloc) : Allocator(alloc) {}
  constexpr explicit TypedChunk(Allocator&& alloc) : Allocator(std::forward<Allocator>(alloc)) {}

  constexpr explicit TypedChunk(Size size)
      : begin_(allocate_memory(*this, size)), end_(begin_ + size) {}
  constexpr TypedChunk(Size size, Allocator&& alloc)
      : Allocator(std::forward<Allocator>(alloc)), begin_(allocate_memory(*this, size)),
        end_(begin_ + size) {}
  constexpr TypedChunk(Size size, const Allocator& alloc)
      : Allocator(alloc), begin_(allocate_memory(*this, size)), end_(begin_ + size) {}

  constexpr TypedChunk(TypedChunk&& other) noexcept
      : Allocator(static_cast<Allocator&&>(other)), begin_(other.begin_), end_(other.end_) {
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

  [[nodiscard]] static constexpr TValue* allocate_memory(Allocator& allocator, const Size size) {
    return std::allocator_traits<Allocator>::allocate(allocator, size);
  }
  void allocate(const Size size) {
    assert(begin_ == nullptr && end_ == nullptr);
    begin_ = allocate_memory(*this, size);
    end_ = begin_ + size;
  }

  // WARNING Only valid if the data is fully initialized!
  constexpr void destroy_initialized() {
    std::destroy(begin_, end_);
  }
  constexpr void deallocate() {
    if (begin_ != nullptr) {
      assert(end_ != nullptr);
      std::allocator_traits<Allocator>::deallocate(*this, begin_, static_cast<Size>(end_ - begin_));
    } else {
      assert(end_ == nullptr);
    }
  }

  void copy_allocator(const Allocator& allocator) {
    static_cast<Allocator&>(*this) = allocator;
  }

  // WARNING Requires the data to be destroyed already!
  void move_to_destroyed(TypedChunk&& other) noexcept {
    deallocate();

    static_cast<Allocator&>(*this) = static_cast<Allocator&&>(other);
    begin_ = other.begin_;
    other.begin_ = nullptr;

    end_ = other.end_;
    other.end_ = nullptr;
  }
  // WARNING Requires the data to be destroyed already!
  void reallocate_to_destroyed(const TypedChunk& other) {
    assert(this != &other);

    deallocate();

    static_cast<Allocator&>(*this) = static_cast<const Allocator&>(other);
    const auto size = static_cast<Size>(other.end_ - other.begin_);
    begin_ = allocate_memory(*this, size);
    end_ = begin_ + size;
  }

  constexpr void allocate_to_empty(Size size) {
    assert(begin_ == nullptr && end_ == nullptr);
    begin_ = allocate_memory(*this, size);
    end_ = begin_ + size;
  }
  // WARNING Only valid if the data is fully initialized!
  template<typename TMover, typename TDestructor>
  constexpr void expand(Size new_size, TMover&& mover, TDestructor&& destructor) {
    assert(new_size > size());

    Value* new_begin = allocate_memory(*this, new_size);
    mover(begin_, end_, new_begin);
    destructor(begin_, end_);
    deallocate();

    begin_ = new_begin;
    end_ = new_begin + new_size;
  }
  constexpr void expand(Size new_size, iterator data_end) {
    expand(
      new_size,
      [data_end](iterator old_begin, [[maybe_unused]] iterator old_end, iterator new_begin) {
        std::uninitialized_move(old_begin, data_end, new_begin);
      },
      [data_end](iterator old_begin, [[maybe_unused]] iterator old_end) {
        std::destroy(old_begin, data_end);
      });
  }
  // WARNING Only valid if the data is fully initialized!
  constexpr void shrink(Size new_size) {
    assert(new_size < size());

    Value* new_begin = allocate_memory(*this, new_size);
    std::uninitialized_move(begin_, begin_ + new_size, new_begin);

    destroy_initialized();
    deallocate();

    begin_ = new_begin;
    end_ = new_begin + new_size;
  }

private:
  TValue* begin_{nullptr};
  TValue* end_{nullptr};
};
} // namespace thes::array

#endif // INCLUDE_THESAUROS_CONTAINERS_ARRAY_TYPED_CHUNK_HPP
