#ifndef INCLUDE_THESAUROS_CONTAINERS_NESTED_DYNAMIC_ARRAY_HPP
#define INCLUDE_THESAUROS_CONTAINERS_NESTED_DYNAMIC_ARRAY_HPP

#include <cassert>
#include <cstddef>
#include <memory>
#include <ostream>
#include <span>
#include <utility>

#include "thesauros/containers/array/typed-chunk.hpp"
#include "thesauros/io/printers.hpp"
#include "thesauros/iterator/facades.hpp"
#include "thesauros/iterator/provider-map.hpp"
#include "thesauros/utility/type-transformations.hpp"

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

  using Storage = array::TypedChunk<TValue, Size, Allocator>;
  using SizeStorage = array::TypedChunk<Size, Size, SizeAllocator>;

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
  struct Iterator : public IteratorFacade<Iterator<tConst>,
                                          iter_provider::Map<IterProv<tConst>, Iterator<tConst>>> {
    friend struct IterProv<tConst>;
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
    return span_impl<false>(values_.begin(), offsets_.begin() + index);
  }
  std::span<const Value> operator[](Size index) const {
    return span_impl<true>(values_.begin(), offsets_.begin() + index);
  }

  friend std::ostream& operator<<(std::ostream& s, const NestedDynamicArrayBase& b) {
    return s << '[' << range_print(b) << ']';
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
  [[nodiscard]] Size element_num() const {
    return values_.size();
  }

private:
  template<bool tConst>
  static std::span<ConditionalConst<tConst, Value>>
  span_impl(ConditionalConst<tConst, Value>* value_begin, const Size* offset_current) {
    return std::span<ConditionalConst<tConst, Value>>(value_begin + offset_current[0],
                                                      value_begin + offset_current[1]);
  }

  SizeStorage offsets_;
  Storage values_;
};

template<typename T>
struct NestedDynamicArray
    : public NestedDynamicArrayBase<NestedDynamicArray<T>, T, std::size_t, std::allocator<T>,
                                    std::allocator<std::size_t>> {
  using Parent = NestedDynamicArrayBase<NestedDynamicArray<T>, T, std::size_t, std::allocator<T>,
                                        std::allocator<std::size_t>>;

  using Parent::Parent;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_NESTED_DYNAMIC_ARRAY_HPP
