// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_CHUNKED_DYNAMIC_ARRAY_HPP
#define INCLUDE_THESAUROS_CONTAINERS_CHUNKED_DYNAMIC_ARRAY_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <memory>
#include <numeric>
#include <span>
#include <type_traits>
#include <utility>

#include "thesauros/containers/array/growth-policy.hpp"
#include "thesauros/containers/array/typed-chunk.hpp"
#include "thesauros/iterator/facades.hpp"
#include "thesauros/iterator/provider-map.hpp"
#include "thesauros/ranges/iota.hpp"
#include "thesauros/types/type-transformations.hpp"

namespace thes {
template<typename TValue, typename TSize>
struct MutableBlock {
  using Value = TValue;

  using value_type = Value;
  using iterator = Value*;
  using const_iterator = const Value*;

  MutableBlock(Value* begin, Value* end, Value* end_of_block, TSize* size)
      : begin_(begin), end_(end), end_of_block_(end_of_block), size_(size) {}

  iterator begin() {
    return begin_;
  }
  iterator end() {
    return end_;
  }
  const_iterator begin() const {
    return begin_;
  }
  const_iterator end() const {
    return end_;
  }

  template<typename... TArgs>
  void emplace_back(TArgs&&... args) {
    assert(end_ != end_of_block_);
    new (end_) Value(std::forward<TArgs>(args)...);
    ++end_;
    ++(*size_);
  }
  void push_back(Value&& value) {
    emplace_back(std::forward<Value>(value));
  }
  void push_back(const Value& value) {
    emplace_back(value);
  }

  void erase(const Value& value) {
    Value* remove_begin = std::remove(begin_, end_, value);
    std::destroy(remove_begin, end_);
    end_ = remove_begin;
    *size_ = static_cast<TSize>(end_ - begin_);
  }

  Value operator[](TSize i) const {
    assert(i < size());
    return begin_[i];
  }

  [[nodiscard]] TSize size() const {
    const auto size = static_cast<TSize>(end_ - begin_);
    assert(size == (*size_));
    return size;
  }

  std::span<const Value> span() const {
    return std::span<const Value>(begin_, end_);
  }
  std::span<Value> span() {
    return std::span<Value>(begin_, end_);
  }

private:
  Value* begin_;
  Value* end_;
  Value* end_of_block_;
  TSize* size_;
};

template<typename TValue, typename TSize, typename TAllocator, typename TSizeAllocator,
         typename TGrowthPolicy>
struct ChunkedDynamicArrayBase {
  using Value = TValue;
  using Size = TSize;
  using Allocator = TAllocator;
  using SizeAllocator = TSizeAllocator;

  using value_type = Value;
  using size_type = Size;

  using Storage = array::TypedChunk<TValue, Size, Allocator>;
  using SizeStorage = array::TypedChunk<Size, Size, SizeAllocator>;

  using GrowthPolicy = TGrowthPolicy;

  explicit ChunkedDynamicArrayBase(Size block_size) : block_size_(block_size) {}
  ChunkedDynamicArrayBase(Size block_size, Allocator&& allocator, SizeAllocator&& size_allocator)
      : sizes_(std::forward<Allocator>(allocator)),
        elements_(std::forward<SizeAllocator>(size_allocator)), block_size_(block_size) {}
  ChunkedDynamicArrayBase(Size block_size, const Allocator& allocator,
                          const SizeAllocator& size_allocator)
      : sizes_(allocator), elements_(size_allocator), block_size_(block_size) {}

  ChunkedDynamicArrayBase(ChunkedDynamicArrayBase&&) noexcept = default;
  ChunkedDynamicArrayBase(const ChunkedDynamicArrayBase&) = delete;
  ChunkedDynamicArrayBase& operator=(ChunkedDynamicArrayBase&&) = delete;
  ChunkedDynamicArrayBase& operator=(const ChunkedDynamicArrayBase&) = delete;
  ~ChunkedDynamicArrayBase() = default;

  using Block = MutableBlock<TValue, TSize>;
  using ConstBlock = std::span<const Value>;

private:
  template<bool tConst>
  struct StateProv {
    using Val = std::conditional_t<tConst, ConstBlock, Block>;

    struct IterTypes : public iter_provider::ValueTypes<Val, std::ptrdiff_t> {
      using IterState = Size;
    };

    static Val deref(const auto& self) {
      using CSize = ConditionalConst<tConst, Size>;
      using CValue = ConditionalConst<tConst, Value>;

      CSize* size_ptr = self.size_begin_ + self.index_;
      const Size size = *size_ptr;
      CValue* block_begin = self.value_begin_ + self.block_size_ * self.index_;
      if constexpr (tConst) {
        return ConstBlock(block_begin, block_begin + size);
      } else {
        return Block(block_begin, block_begin + size, block_begin + self.block_size_, size_ptr);
      }
    }
    static Size& state(auto& self) {
      return self.index_;
    }
    static const Size& state(const auto& self) {
      return self.index_;
    }

    static void test_if_cmp([[maybe_unused]] const auto& i1, [[maybe_unused]] const auto& i2) {
      assert(i1.block_size_ == i2.block_size_);
      assert(i1.size_begin_ == i2.size_begin_);
      assert(i1.value_begin_ == i2.value_begin_);
    }
  };

public:
  template<bool tConst>
  struct Iterator : public IteratorFacade<Iterator<tConst>, iter_provider::Map<StateProv<tConst>>> {
    friend StateProv<tConst>;
    using CSize = ConditionalConst<tConst, Size>;
    using CValue = ConditionalConst<tConst, Value>;

    Iterator() = default;
    Iterator(CSize* size_begin, CValue* value_begin, Size index, Size block_size)
        : index_(index), block_size_(block_size), size_begin_(size_begin),
          value_begin_(value_begin) {}

  private:
    Size index_{};
    Size block_size_{};
    CSize* size_begin_{};
    CValue* value_begin_{};
  };

  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;

  const_iterator begin() const {
    return const_iterator(sizes_.data(), elements_.data(), 0, block_size_);
  }
  iterator begin() {
    return iterator(sizes_.data(), elements_.data(), 0, block_size_);
  }
  const_iterator end() const {
    return const_iterator(sizes_.data(), elements_.data(), block_num_, block_size_);
  }
  iterator end() {
    return iterator(sizes_.data(), elements_.data(), block_num_, block_size_);
  }

  Block operator[](Size i) {
    Size* size_ptr = sizes_.data() + i;
    Value* val_ptr = elements_.data() + (i * block_size_);
    return Block(val_ptr, val_ptr + (*size_ptr), val_ptr + block_size_, size_ptr);
  }
  ConstBlock operator[](Size i) const {
    const Value* val_ptr = elements_.data() + (i * block_size_);
    return ConstBlock(val_ptr, val_ptr + sizes_[i]);
  }

  void add_blocks(Size block_num) {
    assert(elements_.size() % block_size_ == 0);
    assert(sizes_.size() == elements_.size() / block_size_);

    const Size current_allocation = sizes_.size();
    const Size old_block_num = block_num_;
    const Size new_block_num = old_block_num + block_num;

    // Case distinction: Do we need more memory?
    if (new_block_num >= current_allocation) {
      const Size new_allocation_size = grown_size(new_block_num);

      elements_.expand(new_allocation_size * block_size_,
                       [&](Value* old_begin, Value* /*old_end*/, Value* new_begin) {
                         for (const auto i : range(old_block_num)) {
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
  constexpr Size grown_size(Size new_size_lower_bound) const {
    return GrowthPolicy::new_allocation_size(block_num_, new_size_lower_bound);
  }

  Size block_size_;
  Size block_num_{0};
  SizeStorage sizes_{};
  Storage elements_{};
};

template<typename T>
using ChunkedDynamicArray = ChunkedDynamicArrayBase<T, std::size_t, std::allocator<T>,
                                                    std::allocator<std::size_t>, DoublingGrowth>;
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_CHUNKED_DYNAMIC_ARRAY_HPP
