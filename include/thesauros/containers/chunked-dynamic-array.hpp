// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_CHUNKED_DYNAMIC_ARRAY_HPP
#define INCLUDE_THESAUROS_CONTAINERS_CHUNKED_DYNAMIC_ARRAY_HPP

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <numeric>
#include <span>
#include <type_traits>
#include <utility>

#include "thesauros/containers/array/growth-policy.hpp"
#include "thesauros/containers/array/typed-chunk.hpp"
#include "thesauros/iterator/facade.hpp"
#include "thesauros/iterator/state-facade.hpp"
#include "thesauros/math/integer-cast.hpp"
#include "thesauros/ranges/indices.hpp"
#include "thesauros/types/type-transformations.hpp"

namespace thes {
/** A single fixed-capacity block of a `ChunkedDynamicArrayBase`, exposing vector-like access. */
template<typename Value, typename Size>
struct MutableBlock {
  using value_type = Value;
  using size_type = Size;
  using iterator = Value*;
  using const_iterator = const Value*;

  MutableBlock(Value* begin, Value* end, Value* end_of_block, Size* size)
      : begin_(begin), end_(end), end_of_block_(end_of_block), size_(size) {}

  auto begin(this auto& self) {
    return SelfIterator<decltype(self)>{self.begin_};
  }
  auto end(this auto& self) {
    return SelfIterator<decltype(self)>{self.end_};
  }

  /** Constructs a value in place at the end of the block. */
  template<typename... Args>
  void emplace_back(Args&&... args) {
    assert(end_ != end_of_block_);
    new (end_) Value(std::forward<Args>(args)...);
    ++end_;
    ++(*size_);
  }
  void push_back(Value&& value) {
    emplace_back(std::forward<Value>(value));
  }
  void push_back(const Value& value) {
    emplace_back(value);
  }

  /** Removes every element equal to `value`, preserving the relative order of the rest. */
  void erase(const Value& value) {
    Value* remove_begin = std::remove(begin_, end_, value);
    std::destroy(remove_begin, end_);
    end_ = remove_begin;
    *size_ = *safe_cast<Size>(end_ - begin_);
  }

  Value operator[](Size i) const {
    assert(i < size());
    return begin_[i];
  }

  [[nodiscard]] Size size() const {
    const auto size = *safe_cast<Size>(end_ - begin_);
    assert(size == (*size_));
    return size;
  }

  auto span(this auto& self) {
    using Span = std::conditional_t<std::is_const_v<std::remove_reference_t<decltype(self)>>,
                                    std::span<const Value>, std::span<Value>>;
    return Span{self.begin_, self.end_};
  }

private:
  template<typename Self>
  using SelfIterator =
    std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>, const_iterator, iterator>;

  Value* begin_;
  Value* end_;
  Value* end_of_block_;
  Size* size_;
};

/**
 * A dynamic array stored as a sequence of fixed-size blocks, each block growing independently up
 * to `block_size_`.
 */
template<typename Value, typename Size, typename Allocator, typename SizeAllocator,
         typename GrowthPolicy>
requires(std::same_as<typename std::allocator_traits<Allocator>::value_type, Value> &&
         std::same_as<typename std::allocator_traits<SizeAllocator>::value_type, Size>)
struct ChunkedDynamicArrayBase {
  using value_type = Value;
  using size_type = Size;

  using Storage = array::TypedChunk<Value, Size, Allocator>;
  using SizeStorage = array::TypedChunk<Size, Size, SizeAllocator>;

  using Block = MutableBlock<Value, Size>;
  using ConstBlock = std::span<const Value>;

  //------------------------------------------------------------------------------------------------
  // Construction
  //------------------------------------------------------------------------------------------------

  explicit ChunkedDynamicArrayBase(Size block_size) : block_size_(block_size) {}
  ChunkedDynamicArrayBase(Size block_size, Allocator&& allocator, SizeAllocator&& size_allocator)
      : block_size_(block_size), sizes_(std::forward<SizeAllocator>(size_allocator)),
        elements_(std::forward<Allocator>(allocator)) {}
  ChunkedDynamicArrayBase(Size block_size, const Allocator& allocator,
                          const SizeAllocator& size_allocator)
      : block_size_(block_size), sizes_(size_allocator), elements_(allocator) {}

  ChunkedDynamicArrayBase(ChunkedDynamicArrayBase&&) noexcept = default;
  ChunkedDynamicArrayBase(const ChunkedDynamicArrayBase&) = delete;
  ChunkedDynamicArrayBase& operator=(ChunkedDynamicArrayBase&&) = delete;
  ChunkedDynamicArrayBase& operator=(const ChunkedDynamicArrayBase&) = delete;
  ~ChunkedDynamicArrayBase() = default;

  //------------------------------------------------------------------------------------------------
  // Iterator
  //------------------------------------------------------------------------------------------------

  template<bool IsConst>
  struct BaseIterator
      : public StateIteratorFacade<
          iter::ValueTypes<std::conditional_t<IsConst, ConstBlock, Block>, std::ptrdiff_t>> {
    using CBlock = std::conditional_t<IsConst, ConstBlock, Block>;
    using CSize = ConditionalConst<IsConst, Size>;
    using CValue = ConditionalConst<IsConst, Value>;

    friend StateIteratorFacade<iter::ValueTypes<CBlock, std::ptrdiff_t>>;

    BaseIterator() = default;
    BaseIterator(CSize* size_begin, CValue* value_begin, Size index, Size block_size)
        : index_(index), block_size_(block_size), size_begin_(size_begin),
          value_begin_(value_begin) {}

  private:
    CBlock value() const {
      CSize* size_ptr = size_begin_ + index_;
      const Size size = *size_ptr;
      CValue* block_begin = value_begin_ + block_size_ * index_;
      if constexpr (IsConst) {
        return ConstBlock{block_begin, block_begin + size};
      } else {
        return Block{block_begin, block_begin + size, block_begin + block_size_, size_ptr};
      }
    }
    auto& state(this auto& self) {
      return self.index_;
    }

    void test_if_cmp([[maybe_unused]] const auto& other) const {
      assert(block_size_ == other.block_size_);
      assert(size_begin_ == other.size_begin_);
      assert(value_begin_ == other.value_begin_);
    }

    Size index_{};
    Size block_size_{};
    CSize* size_begin_{};
    CValue* value_begin_{};
  };

  using iterator = BaseIterator<false>;
  using const_iterator = BaseIterator<true>;

  //------------------------------------------------------------------------------------------------
  // Element access
  //------------------------------------------------------------------------------------------------

  auto begin(this auto& self) {
    return SelfIterator<decltype(self)>{self.sizes_.data(), self.elements_.data(), Size{0},
                                        self.block_size_};
  }
  auto end(this auto& self) {
    return SelfIterator<decltype(self)>{self.sizes_.data(), self.elements_.data(), self.block_num_,
                                        self.block_size_};
  }

  auto operator[](this auto& self, Size i) {
    if constexpr (std::is_const_v<std::remove_reference_t<decltype(self)>>) {
      const Value* val_ptr = self.elements_.data() + (i * self.block_size_);
      return ConstBlock{val_ptr, val_ptr + self.sizes_[i]};
    } else {
      Size* size_ptr = self.sizes_.data() + i;
      Value* val_ptr = self.elements_.data() + (i * self.block_size_);
      return Block{val_ptr, val_ptr + (*size_ptr), val_ptr + self.block_size_, size_ptr};
    }
  }

  //------------------------------------------------------------------------------------------------
  // Block management
  //------------------------------------------------------------------------------------------------

  /** Appends `block_num` empty blocks, growing the backing storage if necessary. */
  void add_blocks(Size block_num) {
    assert(elements_.size() % block_size_ == 0);
    assert(sizes_.size() == elements_.size() / block_size_);

    if (block_num == Size{0}) {
      return;
    }

    const Size current_allocation = sizes_.size();
    const Size old_block_num = block_num_;
    const Size new_block_num = old_block_num + block_num;

    // Case distinction: Do we need more memory?
    if (new_block_num > current_allocation) {
      const Size new_allocation_size = grown_size(new_block_num);

      elements_.expand(new_allocation_size * block_size_,
                       [&](Value* old_begin, Value* /*old_end*/, Value* new_begin) {
                         for (const auto i : views::indices(old_block_num)) {
                           const auto offset = i * block_size_;

                           auto b = old_begin + offset;
                           auto e = b + sizes_[i];

                           std::uninitialized_move(b, e, new_begin + offset);
                           std::destroy(b, e);
                         }
                       });

      Size* old_size_end = sizes_.data() + old_block_num;
      sizes_.expand(new_allocation_size, [&](Size* old_begin, Size* /*old_end*/, Size* new_begin) {
        std::uninitialized_move(old_begin, old_size_end, new_begin);
        std::uninitialized_fill(new_begin + old_block_num, new_begin + new_block_num, Size{0});
        std::destroy(old_begin, old_size_end);
      });
    } else {
      Size* size_data = sizes_.data();
      std::uninitialized_fill(size_data + old_block_num, size_data + new_block_num, Size{0});
    }

    block_num_ = new_block_num;
  }
  void push_block() {
    add_blocks(1);
  }
  void pop_block() {
    assert(block_num_ > 0);

    const Size last = block_num_ - 1;
    Size* last_size_ptr = sizes_.data() + last;
    const Size last_size = *last_size_ptr;
    std::destroy_at(last_size_ptr);

    Value* last_block_begin = elements_.data() + last * block_size_;
    std::destroy(last_block_begin, last_block_begin + last_size);

    --block_num_;
  }

  [[nodiscard]] Size block_num() const {
    return block_num_;
  }
  [[nodiscard]] Size alloc_size() const {
    return block_num_ * block_size_;
  }
  [[nodiscard]] Size value_num() const {
    return std::reduce(sizes_.begin(), sizes_.begin() + block_num_, Size{0}, std::plus<>{});
  }

private:
  template<typename Self>
  using SelfIterator = BaseIterator<std::is_const_v<std::remove_reference_t<Self>>>;

  constexpr Size grown_size(Size new_size_lower_bound) const {
    return GrowthPolicy::new_allocation_size(block_num_, new_size_lower_bound);
  }

  Size block_size_;
  Size block_num_{0};
  SizeStorage sizes_{};
  Storage elements_{};
};

template<typename Value>
using ChunkedDynamicArray = ChunkedDynamicArrayBase<Value, std::size_t, std::allocator<Value>,
                                                    std::allocator<std::size_t>, DoublingGrowth>;
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_CHUNKED_DYNAMIC_ARRAY_HPP
