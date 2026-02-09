// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_NESTED_DYNAMIC_ARRAY_HPP
#define INCLUDE_THESAUROS_CONTAINERS_NESTED_DYNAMIC_ARRAY_HPP

#include <cassert>
#include <cstddef>
#include <memory>
#include <span>
#include <utility>

#include "thesauros/containers/array/typed-chunk.hpp"
#include "thesauros/io.hpp"
#include "thesauros/iterator/facades.hpp"
#include "thesauros/iterator/provider-map.hpp"
#include "thesauros/ranges/iota.hpp"
#include "thesauros/types/type-transformations.hpp"

namespace thes {
template<typename TDerived, typename TValue, typename TSize, typename TAllocator,
         typename TSizeAllocator>
struct NestedDynamicArrayBase {
  using Derived = TDerived;
  using Value = TValue;
  using Size = TSize;
  using Allocator = TAllocator;
  using SizeAllocator = TSizeAllocator;

  using value_type = Value;
  using size_type = Size;

  using SizeStorage = array::TypedChunk<Size, Size, SizeAllocator>;
  using Storage = array::TypedChunk<TValue, Size, Allocator>;

  static TDerived from_file(FileReader& reader) {
    auto offsets = SizeStorage::from_file(reader);
    auto values = Storage::from_file(reader);
    return TDerived{std::move(offsets), std::move(values)};
  }

private:
  template<bool tConst>
  struct IterProv {
    using Val = std::span<ConditionalConst<tConst, Value>>;
    using State = Size;

    struct IterTypes : public iter_provider::ValueTypes<Val, std::ptrdiff_t> {
      using IterState = State;
    };

    static Val deref(const auto& self) {
      return span_impl<tConst>(self.value_begin_, self.offset_begin_ + self.index_);
    }
    static State& state(auto& self) {
      return self.index_;
    }
    static const State& state(const auto& self) {
      return self.index_;
    }

    static void test_if_cmp([[maybe_unused]] const auto& i1, [[maybe_unused]] const auto& i2) {
      assert(i1.offset_begin_ == i2.offset_begin_);
      assert(i1.value_begin_ == i2.value_begin_);
    }
  };

public:
  template<bool tConst>
  struct Iterator : public IteratorFacade<Iterator<tConst>, iter_provider::Map<IterProv<tConst>>> {
    friend IterProv<tConst>;
    using CValue = ConditionalConst<tConst, Value>;
    using CSpan = std::span<CValue>;

    Iterator(const Size* offset_begin, CValue* value_begin, Size index)
        : index_(index), offset_begin_(offset_begin), value_begin_(value_begin) {}

  private:
    Size index_;
    const Size* offset_begin_;
    CValue* value_begin_;
  };

  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;

  const_iterator begin() const {
    return const_iterator(offsets_.begin(), values_.begin(), 0);
  }
  iterator begin() {
    return iterator(offsets_.begin(), values_.begin(), 0);
  }
  const_iterator end() const {
    return const_iterator(offsets_.begin(), values_.begin(), group_num());
  }
  iterator end() {
    return iterator(offsets_.begin(), values_.begin(), group_num());
  }

  std::span<Value> operator[](Size index) {
    assert(index + 1 < offsets_.size());
    return span_impl<false>(values_.begin(), offsets_.begin() + index);
  }
  std::span<const Value> operator[](Size index) const {
    assert(index + 1 < offsets_.size());
    return span_impl<true>(values_.begin(), offsets_.begin() + index);
  }

  // element access: outer groups
  std::span<value_type> front() {
    assert(!empty());
    return (*this)[0];
  }
  std::span<const value_type> front() const {
    assert(!empty());
    return (*this)[0];
  }

  std::span<value_type> back() {
    assert(!empty());
    return (*this)[size() - 1];
  }
  std::span<const value_type> back() const {
    assert(!empty());
    return (*this)[size() - 1];
  }

  struct FlatBuilder {
    FlatBuilder() = default;

    void initialize(Size group_num, Size element_num) {
      offsets_.allocate_to_empty(group_num + 1);
      offsets_.front() = 0;
      offsets_current_ = offsets_.begin() + 1;

      values_.allocate_to_empty(element_num);
      values_current_ = values_.begin();
    }

    template<typename... TArgs>
    void emplace(TArgs&&... args) {
      new (values_current_) Value(std::forward<TArgs>(args)...);
      ++values_current_;
    }

    void advance_group() {
      ++offsets_current_;
      new (offsets_current_) Size(values_current_ - values_.begin());
    }

    Derived build() {
      assert(offsets_current_ == offsets_.end());
      assert(values_current_ == values_.end());
      return Derived(std::move(offsets_), std::move(values_));
    }

  private:
    SizeStorage offsets_{};
    Storage values_{};
    Size* offsets_current_{nullptr};
    Value* values_current_{nullptr};
  };

  struct NestedBuilderInner {
    NestedBuilderInner(Size* offsets_current, const Value* values_begin, Value* values_current)
        : offsets_current_(offsets_current), values_begin_(values_begin),
          values_current_(values_current) {}

    template<typename... TArgs>
    void emplace(TArgs&&... args) {
      new (values_current_) Value(std::forward<TArgs>(args)...);
      ++values_current_;
    }

    void advance_group() {
      ++offsets_current_;
      const auto size = static_cast<Size>(values_current_ - values_begin_);
      new (offsets_current_) Size(size);
    }

  private:
    Size* offsets_current_;
    const Value* values_begin_;
    Value* values_current_;
  };

  struct NestedBuilder {
    NestedBuilder() = default;

    void initialize(Size group_num, Size element_num) {
      offsets_.allocate_to_empty(group_num + 1);
      offsets_.front() = 0;
      values_.allocate_to_empty(element_num);
    }

    NestedBuilderInner part_builder(Size nested_from, Size full_from) {
      return NestedBuilderInner(offsets_.begin() + nested_from, values_.begin(),
                                values_.begin() + full_from);
    }

    Derived build() {
      return Derived(std::move(offsets_), std::move(values_));
    }

  private:
    SizeStorage offsets_{};
    Storage values_{};
  };

  NestedDynamicArrayBase(SizeStorage&& offsets, Storage&& values)
      : offsets_(std::forward<SizeStorage>(offsets)), values_(std::forward<Storage>(values)) {
    assert(!offsets_.empty());
  }

  [[nodiscard]] Size group_num() const {
    assert(!offsets_.empty());
    return offsets_.size() - 1;
  }
  [[nodiscard]] Size size() const noexcept {
    return group_num();
  }
  [[nodiscard]] bool empty() const noexcept {
    return size() == 0;
  }

  [[nodiscard]] Size element_num() const {
    return values_.size();
  }
  [[nodiscard]] Size flat_size() const noexcept {
    return element_num();
  }

  void to_file(FileWriter& writer) const {
    offsets_.to_file(writer);
    values_.to_file(writer);
  }

  IotaRange<Size> offsets_of(Size i) const {
    return range(offsets_[i], offsets_[i + 1]);
  }

private:
  template<bool tConst>
  static std::span<ConditionalConst<tConst, Value>>
  span_impl(ConditionalConst<tConst, Value>* value_begin, const Size* offset_current) {
    assert(offset_current[0] <= offset_current[1]);
    return std::span<ConditionalConst<tConst, Value>>(value_begin + offset_current[0],
                                                      value_begin + offset_current[1]);
  }

  SizeStorage offsets_;
  Storage values_;
};

template<typename T, typename TSize, typename TAlloc = std::allocator<T>>
struct NestedDynamicArray
    : public NestedDynamicArrayBase<
        NestedDynamicArray<T, TSize, TAlloc>, T, TSize, TAlloc,
        typename std::allocator_traits<TAlloc>::template rebind_alloc<TSize>> {
  using Parent =
    NestedDynamicArrayBase<NestedDynamicArray<T, TSize, TAlloc>, T, TSize, TAlloc,
                           typename std::allocator_traits<TAlloc>::template rebind_alloc<TSize>>;

  using Parent::Parent;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_NESTED_DYNAMIC_ARRAY_HPP
