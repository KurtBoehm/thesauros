// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_MULTI_BYTE_INTEGERS_HPP
#define INCLUDE_THESAUROS_CONTAINERS_MULTI_BYTE_INTEGERS_HPP

#include <algorithm>
#include <bit>
#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <memory>
#include <span>
#include <type_traits>

#include "thesauros/containers/array/dynamic.hpp"
#include "thesauros/containers/array/growth-policy.hpp"
#include "thesauros/containers/array/initialization-policy.hpp"
#include "thesauros/io.hpp"
#include "thesauros/iterator/facade.hpp"
#include "thesauros/iterator/reverse-facade.hpp"
#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/types/type-tag.hpp"
#include "thesauros/types/type-transformations.hpp"
#include "thesauros/utility/integral-value.hpp"
#include "thesauros/utility/value-optional.hpp"

namespace thes {
namespace impl {
/**
 * Owns a byte buffer holding a packed array of `ByteInt::byte_num`-byte integers, with
 * `PaddingBytes` bytes of padding at each end so that a full `Unsigned`-width load or store never
 * reads or writes out of bounds near the boundaries.
 */
template<typename ByteInt, std::size_t PaddingBytes, typename ByteAlloc>
struct ArrayStorage {
  using Size = std::size_t;

  static constexpr std::size_t padding_bytes = PaddingBytes;
  static constexpr std::size_t element_bytes = ByteInt::byte_num;

  using Data = DynamicArray<std::byte, DefaultInit, DoublingGrowth, ByteAlloc>;

  /** Creates an empty storage, allocating only the padding. */
  ArrayStorage() : data_(2 * padding_bytes) {};
  /** Creates a storage for `size` elements, allocating padding on both sides. */
  explicit ArrayStorage(std::size_t size) : data_(effective_allocation(size)), size_(size) {}

  /** Returns the byte span of the stored elements, excluding padding, mutable if `self` is. */
  [[nodiscard]] auto span(this auto&& self) {
    return std::span{self.data_.data() + padding_bytes, self.size_ * element_bytes};
  }

  /** Returns the underlying padded byte array, mutable if `self` is. */
  [[nodiscard]] auto& array(this auto&& self) {
    return self.data_;
  }

  /** Returns the number of stored elements, as a mutable reference if `self` is mutable. */
  [[nodiscard]] decltype(auto) size(this auto&& self) {
    return (self.size_);
  }

  /** Computes the byte allocation, padding included, needed to store `allocation` elements. */
  static Size effective_allocation(Size allocation) THES_ALWAYS_INLINE {
    return (allocation * element_bytes) + (2 * padding_bytes);
  }

private:
  Data data_{};
  Size size_{0};
};

/**
 * A non-owning, possibly const, view of a byte buffer holding a packed array of
 * `ByteInt::byte_num`-byte integers.
 */
template<bool IsConst, typename ByteInt>
struct ViewStorage {
  using CByte = ConditionalConst<IsConst, std::byte>;
  using Size = std::size_t;
  static constexpr std::size_t element_bytes = ByteInt::byte_num;

  /** Creates a view of `size` elements starting at `data`. */
  ViewStorage(CByte* data, Size size) : data_(data), size_(size) {}

  /** Returns the byte span of the viewed elements, const if `IsConst` or `self` is const. */
  [[nodiscard]] auto span(this auto&& self) {
    using Byte = ConditionalConst<IsConst || ConstAccess<decltype(self)>, std::byte>;
    return std::span<Byte>{self.data_, self.size_ * element_bytes};
  }

  /** Returns the number of viewed elements. */
  [[nodiscard]] Size size() const {
    return size_;
  }

private:
  CByte* data_{};
  Size size_{0};
};
} // namespace impl

/** A const or mutable, optionally value-optional, view of a range of packed `ByteInt` integers. */
template<bool IsConst, typename ByteInt, std::size_t PaddingBytes, bool IsOptional>
struct MultiByteSubRange;

/**
 * The CRTP base shared by `MultiByteSubRange` and `MultiByteIntegerArray`, implementing the shared
 * container interface, i.e. iteration, element access, sub-ranging and serialization, in terms of a
 * `Storage` type that owns or views the underlying packed byte buffer. `Derived` must be either a
 * `MultiByteSubRange` or a `MultiByteIntegerArray` instantiation.
 */
template<typename Derived, typename ByteInt, std::size_t PaddingBytes, bool IsOptional,
         typename Storage>
struct MultiByteIntegersBase {
  static_assert(std::endian::native == std::endian::little ||
                  std::endian::native == std::endian::big,
                "Only big and little endian systems are supported!");

  using BaseValue = ByteInt::Unsigned;
  static constexpr BaseValue int_bytes = sizeof(BaseValue);
  static constexpr BaseValue mask = ByteInt::max;
  /** Whether elements are immutable. */
  static constexpr bool is_const =
    std::is_const_v<std::remove_pointer_t<decltype(std::declval<Storage>().span().data())>>;
  /** Whether elements accessed through a `Self`-typed reference are immutable. */
  template<typename Self>
  static constexpr bool const_access = is_const || ConstAccess<Self>;

  /** The element type: `BaseValue` directly, or `ValueOptional<BaseValue, mask>` if optional. */
  using Value = std::conditional_t<IsOptional, ValueOptional<BaseValue, mask>, BaseValue>;
  using Size = std::size_t;

  static constexpr std::size_t padding_bytes = PaddingBytes;
  static constexpr std::size_t element_bytes = ByteInt::byte_num;
  static constexpr std::size_t overhead_bits = ByteInt::overhead_bit_num;

  static_assert(element_bytes <= int_bytes);
  static_assert(padding_bytes >= int_bytes);

  using value_type = Value;
  using size_type = Size;

  //------------------------------------------------------------------------------------------------
  // Element reference
  //------------------------------------------------------------------------------------------------

  /**
   * A proxy reference to a single packed integer, allowing it to be read as a `Value` and assigned
   * through, similar to `std::vector<bool>::reference`.
   */
  struct IntRef {
    explicit IntRef(std::byte* ptr) : ptr_(ptr) {}
    IntRef(const IntRef&) = delete;
    IntRef(IntRef&&) noexcept = default;
    ~IntRef() = default;

    // These are required to be const for iterators to support std::indirectly_writable,
    // which is required for C++20 ranges, e.g. `std::ranges::sort`.
    const IntRef& operator=(const IntRef& ref) const { // NOLINT
      return *this = Value{ref};
    }
    const IntRef& operator=(IntRef&& ref) const noexcept { // NOLINT
      return *this = Value{ref};
    }
    /** Stores `value` at the referenced location. */
    const IntRef& operator=(Value value) const { // NOLINT
      if constexpr (!IsOptional) {
        assert(value == (value & mask));
      }
      store(ptr_, value);
      return *this;
    }

    /** Loads the referenced value. */
    operator Value() const { // NOLINT
      return load(ptr_);
    }

    /** Exchanges the values referenced by `vw1` and `vw2`. */
    friend void swap(IntRef vw1, IntRef vw2) noexcept {
      Value v1 = vw1;
      Value v2 = vw2;
      vw1 = v2;
      vw2 = v1;
    }

    /** Exchanges the value referenced by `vw1` with `i2`. */
    friend void swap(IntRef vw1, Value& i2) noexcept {
      Value v1 = vw1;
      vw1 = i2;
      i2 = v1;
    }
    /** Exchanges `i1` with the value referenced by `vw2`. */
    friend void swap(Value& i1, IntRef vw2) noexcept {
      Value v2 = vw2;
      vw2 = i1;
      i1 = v2;
    }

  private:
    std::byte* ptr_;
  };

  /** The `IteratorFacade` types for `BaseIterator<IsConst>`. */
  template<bool IsConst>
  using IterTypes =
    iter::ValueRefTypes<Value, std::conditional_t<IsConst, Value, IntRef>, std::ptrdiff_t>;

  //------------------------------------------------------------------------------------------------
  // Iterators
  //------------------------------------------------------------------------------------------------

  /** A random-access iterator over the packed integers, const if `IsConst` is `true`. */
  template<bool IsConst>
  struct BaseIterator : public IteratorFacade<IterTypes<IsConst>> {
    using Container = Derived;
    using Ref = IterTypes<IsConst>::IterRef;
    using Diff = IterTypes<IsConst>::IterDiff;
    using Ptr = std::conditional_t<IsConst, const std::byte, std::byte>*;

    friend IteratorFacade<IterTypes<IsConst>>;

    explicit BaseIterator() = default;
    /** Creates an iterator pointing at the element starting at `ptr`. */
    explicit BaseIterator(Ptr ptr) : ptr_{ptr} {}

    /** Returns the underlying byte pointer. */
    [[nodiscard]] Ptr raw() const {
      return ptr_;
    }
    /** Converts a mutable iterator to a const iterator. */
    operator BaseIterator<true>() const { // NOLINT
      return BaseIterator<true>{ptr_};
    }

  private:
    Ref deref() const {
      assert(ptr_ != nullptr);
      if constexpr (IsConst) {
        return load(ptr_);
      } else {
        return Ref{ptr_};
      }
    }

    void incr() {
      ptr_ += element_bytes;
    }
    void decr() {
      ptr_ -= element_bytes;
    }

    void iadd(auto d) {
      ptr_ += byte_size(d);
    }
    void isub(auto d) {
      ptr_ -= byte_size(d);
    }

    bool eq(const BaseIterator& other) const {
      return ptr_ == other.ptr_;
    }
    std::strong_ordering three_way(const BaseIterator& other) const {
      return ptr_ <=> other.ptr_;
    }

    Diff sub(const auto& other) const {
      static constexpr Diff eb = element_bytes;
      Diff diff = ptr_ - other.ptr_;
      assert(diff % eb == 0);
      return diff / eb;
    }

    /** Converts an element offset `d` to a byte offset. */
    template<typename T>
    static auto byte_size(T d) {
      using Integral = IntegralValue<T>;
      using Ret = std::conditional_t<std::unsigned_integral<Integral>, Size, Diff>;
      return Ret{d} * Ret{element_bytes};
    }

    Ptr ptr_{nullptr};
  };

  /**
   * A random-access reverse iterator built from `BaseIterator<IsConst>` via
   * `ReverseIteratorFacade`.
   */
  template<bool IsConst>
  struct BaseReverseIterator : public ReverseIteratorFacade<IterTypes<IsConst>> {
    using ForwardIter = BaseIterator<IsConst>;
    using Container = Derived;
    using Ptr = std::conditional_t<IsConst, const std::byte, std::byte>*;

    friend ReverseIteratorFacade<IterTypes<IsConst>>;

    explicit BaseReverseIterator() = default;
    /**
     * Creates a reverse iterator wrapping the forward iterator pointing at `ptr`.
     * Note that dereferencing always decrements before fetching the value.
     */
    explicit BaseReverseIterator(Ptr ptr) : base_{ForwardIter{ptr}} {}

    /** Converts a mutable reverse iterator to a const reverse iterator. */
    operator BaseReverseIterator<true>() const { // NOLINT
      return BaseReverseIterator<true>{this->base().raw()};
    }

  private:
    auto& base(this auto&& self) {
      return self.base_;
    }

    ForwardIter base_;
  };

  using ConstSubRange = MultiByteSubRange<true, ByteInt, PaddingBytes, IsOptional>;
  using MutableSubRange = MultiByteSubRange<false, ByteInt, PaddingBytes, IsOptional>;

  using iterator = BaseIterator<false>;
  using const_iterator = BaseIterator<true>;
  using reverse_iterator = BaseReverseIterator<false>;
  using const_reverse_iterator = BaseReverseIterator<true>;

  /** Wraps `storage` without copying its contents. */
  explicit MultiByteIntegersBase(Storage&& storage) : storage_(std::forward<Storage>(storage)) {};

  //------------------------------------------------------------------------------------------------
  // Iteration
  //------------------------------------------------------------------------------------------------

  /** Returns an iterator to the first element, mutable if `self` allows it. */
  auto begin(this auto&& self) {
    if constexpr (const_access<decltype(self)>) {
      return const_iterator(self.span().data());
    } else {
      return iterator(self.span().data());
    }
  }
  /** Returns a const iterator to the first element. */
  const_iterator cbegin() const {
    return const_iterator(span().data());
  }

  /** Returns an iterator past the last element, mutable if `self` allows it. */
  auto end(this auto&& self) {
    if constexpr (const_access<decltype(self)>) {
      return const_iterator(self.span().data() + byte_size(self.size()));
    } else {
      return iterator(self.span().data() + byte_size(self.size()));
    }
  }
  /** Returns a const iterator past the last element. */
  const_iterator cend() const {
    return const_iterator(span().data() + byte_size(storage_.size()));
  }

  /** Returns a reverse iterator to the last element, mutable if `self` allows it. */
  auto rbegin(this auto&& self) {
    if constexpr (const_access<decltype(self)>) {
      return const_reverse_iterator(self.span().data() + byte_size(self.size()));
    } else {
      return reverse_iterator(self.span().data() + byte_size(self.size()));
    }
  }
  /** Returns a const reverse iterator to the last element. */
  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(span().data() + byte_size(storage_.size()));
  }

  /** Returns a reverse iterator preceding the first element, mutable if `self` allows it. */
  auto rend(this auto&& self) {
    if constexpr (const_access<decltype(self)>) {
      return const_reverse_iterator(self.span().data());
    } else {
      return reverse_iterator(self.span().data());
    }
  }
  /** Returns a const reverse iterator preceding the first element. */
  const_reverse_iterator crend() const {
    return const_reverse_iterator(span().data());
  }

  //------------------------------------------------------------------------------------------------
  // Size and element access
  //------------------------------------------------------------------------------------------------

  /** Returns the number of stored elements. */
  [[nodiscard]] Size size() const {
    return storage_.size();
  }
  /** Returns whether the container holds no elements. */
  [[nodiscard]] bool empty() const {
    return storage_.size() == 0;
  }

  /** Returns the element at index `i`, as a mutable reference if `self` allows it. */
  decltype(auto) operator[](this auto&& self, Size i) {
    assert(i < self.size());
    if constexpr (const_access<decltype(self)>) {
      return load(self.span().data() + byte_size(i));
    } else {
      return IntRef{self.span().data() + byte_size(i)};
    }
  }

  /** Returns the first element, as a mutable reference if `self` allows it. */
  decltype(auto) front(this auto&& self) {
    assert(self.size() > 0);
    if constexpr (const_access<decltype(self)>) {
      return load(self.span().data());
    } else {
      return IntRef{self.span().data()};
    }
  }

  /** Returns the last element, as a mutable reference if `self` allows it. */
  decltype(auto) back(this auto&& self) {
    assert(self.size() > 0);
    if constexpr (const_access<decltype(self)>) {
      return load(self.span().data() + byte_size(self.size() - 1));
    } else {
      return IntRef{self.span().data() + byte_size(self.size() - 1)};
    }
  }

  /** Returns the raw byte span of the stored elements, mutable if `self` is. */
  [[nodiscard]] auto byte_span(this auto&& self) {
    return std::span{self.span().begin(), byte_size(self.size())};
  }

  //------------------------------------------------------------------------------------------------
  // Sub-ranges
  //------------------------------------------------------------------------------------------------

  /** Returns a view of the half-open index range `[begin, end)`, mutable if `self` is. */
  auto sub_range(this auto&& self, Size begin, Size end) {
    using Result = std::conditional_t<const_access<decltype(self)>, ConstSubRange, MutableSubRange>;
    assert(end >= begin);
    return Result(self.span().data() + byte_size(begin), end - begin);
  }

  /** Returns a view of the entire range, mutable if `self` is. */
  auto full_sub_range(this auto&& self) {
    return self.sub_range(0, self.size());
  }

  //------------------------------------------------------------------------------------------------
  // Serialization
  //------------------------------------------------------------------------------------------------

  /** Writes the element count followed by the raw byte content to `writer`. */
  void to_file(FileWriter& writer) const {
    const Size stored_size = storage_.size();
    writer.write(std::span{&stored_size, 1});
    writer.write(byte_span());
  }

protected:
  /** Converts an element count to a byte count. */
  static Size byte_size(Size size) THES_ALWAYS_INLINE {
    return size * element_bytes;
  }

  /** Loads and unpacks the value stored at `ptr`. */
  static Value load(const std::byte* ptr) THES_ALWAYS_INLINE {
    BaseValue output;
    std::memcpy(&output, ptr, int_bytes);
    if constexpr (std::endian::native == std::endian::little) {
      return output & mask;
    }
    if constexpr (std::endian::native == std::endian::big) {
      return output >> overhead_bits;
    }
    return output;
  }

  /** Shifts `value` into position for a full-width store, in place. */
  static Value& store_transform(Value& value) noexcept THES_ALWAYS_INLINE {
    if constexpr (std::endian::native == std::endian::big) {
      value <<= overhead_bits;
    }
    return value;
  }
  /** Packs and stores `value` at `ptr`, writing exactly `element_bytes` bytes. */
  static void store(std::byte* ptr, Value value) noexcept THES_ALWAYS_INLINE {
    std::memcpy(ptr, &store_transform(value), element_bytes);
  }
  /** Packs and stores `value` at `ptr`, writing a full `int_bytes`-byte word. */
  static void store_full(std::byte* ptr, Value value) THES_ALWAYS_INLINE {
    std::memcpy(ptr, &store_transform(value), int_bytes);
  }

  /** Returns the underlying storage. */
  auto& storage(this auto&& self) {
    return self.storage_;
  }

  /** Returns the byte span exposed by the storage. */
  [[nodiscard]] auto span(this auto&& self) {
    return self.storage_.span();
  }

private:
  Storage storage_;
};

/**
 * A const or mutable, non-owning view of a contiguous range of packed `ByteInt` integers, backed
 * by `impl::ViewStorage`.
 */
template<bool IsConst, typename ByteInt, std::size_t PaddingBytes, bool IsOptional>
struct MultiByteSubRange
    : public MultiByteIntegersBase<MultiByteSubRange<IsConst, ByteInt, PaddingBytes, IsOptional>,
                                   ByteInt, PaddingBytes, IsOptional,
                                   impl::ViewStorage<IsConst, ByteInt>> {
  using Storage = impl::ViewStorage<IsConst, ByteInt>;
  using Base = MultiByteIntegersBase<MultiByteSubRange, ByteInt, PaddingBytes, IsOptional, Storage>;

  using Size = Base::Size;
  using CByte = Storage::CByte;

  /** Creates a view of `size` elements starting at `data`. */
  MultiByteSubRange(CByte* data, Size size) : Base(Storage{data, size}) {}
};

/**
 * An owning, growable array of packed `ByteInt::byte_num`-byte integers, backed by
 * `impl::ArrayStorage`. If `IsOptional` is `true`, elements are `ValueOptional`s that can hold a
 * distinguished “empty” state in addition to any representable value.
 */
template<typename ByteInt, std::size_t PaddingBytes, bool IsOptional, typename ByteAlloc>
struct MultiByteIntegerArray
    : public MultiByteIntegersBase<
        MultiByteIntegerArray<ByteInt, PaddingBytes, IsOptional, ByteAlloc>, ByteInt, PaddingBytes,
        IsOptional, impl::ArrayStorage<ByteInt, PaddingBytes, ByteAlloc>> {
  using Storage = impl::ArrayStorage<ByteInt, PaddingBytes, ByteAlloc>;
  using Base =
    MultiByteIntegersBase<MultiByteIntegerArray, ByteInt, PaddingBytes, IsOptional, Storage>;
  friend Base;

  using Size = Base::Size;
  using Value = Base::Value;
  using iterator = Base::iterator;
  using const_iterator = Base::const_iterator;
  using Base::element_bytes;
  using Base::padding_bytes;

  //------------------------------------------------------------------------------------------------
  // Factory functions
  //------------------------------------------------------------------------------------------------

  /** Reads an array previously written by `to_file` from `reader`. */
  static MultiByteIntegerArray from_file(FileReader& reader) {
    MultiByteIntegerArray out(reader.read(type_tag<Size>));
    reader.read(out.byte_span());
    return out;
  }

  /** Creates an array of `size` elements, with every bit set. */
  static MultiByteIntegerArray create_all_set(std::size_t size)
  requires(!IsOptional)
  {
    MultiByteIntegerArray mbi(size);
    std::fill_n(mbi.span().data(), byte_size(mbi.size()),
                std::byte{std::numeric_limits<unsigned char>::max()});
    return mbi;
  }
  /** Creates an array of `size` elements, all in the empty (unset) `ValueOptional` state. */
  static MultiByteIntegerArray create_empty(std::size_t size)
  requires(IsOptional)
  {
    MultiByteIntegerArray mbi(size);
    std::fill_n(mbi.span().data(), byte_size(mbi.size()),
                std::byte{std::numeric_limits<unsigned char>::max()});
    return mbi;
  }

  /** Creates an array of `size` elements, with every bit cleared. */
  static MultiByteIntegerArray create_zero(std::size_t size) {
    MultiByteIntegerArray mbi(size);
    std::fill_n(mbi.span().data(), byte_size(mbi.size()), std::byte{0});
    return mbi;
  }

  //------------------------------------------------------------------------------------------------
  // Constructors
  //------------------------------------------------------------------------------------------------

  /** Creates an empty array. */
  MultiByteIntegerArray() : Base(Storage{}) {};
  /** Creates an array of `size` default-initialized elements. */
  explicit MultiByteIntegerArray(std::size_t size) : Base(Storage{size}) {}
  /** Creates an array containing the elements of `init`, in order. */
  MultiByteIntegerArray(std::initializer_list<Value> init) : Base(Storage{init.size()}) {
    std::copy(init.begin(), init.end(), this->begin());
  };

  //------------------------------------------------------------------------------------------------
  // Modification
  //------------------------------------------------------------------------------------------------

  /** Appends `value` as the new last element, growing the array if necessary. */
  void push_back(Value value) {
    const Size size = byte_size(storage().size());
    assert(array().size() == size + 2 * padding_bytes);

    array().expand(array().size() + element_bytes);
    ++storage().size();

    this->store_full(span().data() + size, value);
  }

  /** Removes the last element. */
  void pop_back() {
    assert(array().size() == Storage::effective_allocation(storage().size()));
    --storage().size();
    array().shrink(array().size() - element_bytes);
  }

  /** Ensures the array can hold `allocation` elements without reallocating. */
  void reserve(Size allocation) {
    array().reserve(Storage::effective_allocation(allocation));
  }

  /** Sets every bit of every element in the half-open index range `[first, last)`. */
  void set_all(Size first, Size last) {
    std::byte* ptr = span().data();
    std::fill(ptr + byte_size(first), ptr + byte_size(last),
              std::byte{std::numeric_limits<unsigned char>::max()});
  }

  /**
   * Makes room for `ins_size` uninitialized elements before `pos`, additionally growing the array
   * by `pad_end` uninitialized trailing elements, and returns an iterator to the first newly
   * inserted element.
   */
  iterator insert_any(const_iterator pos, Size ins_size, Size pad_end = 0) {
    const std::ptrdiff_t offset = pos.raw() - span().data();
    const Size old_bsize = byte_size(storage().size());
    assert(array().size() == old_bsize + 2 * padding_bytes);

    const Size size = ins_size + pad_end;
    if (size == 0) {
      return iterator{span().data() + offset};
    }
    const Size ins_bsize = byte_size(ins_size);
    const Size pad_bsize = byte_size(pad_end);

    // Reserve once so neither `insert_any` call below reallocates a second time.
    array().reserve(old_bsize + 2 * padding_bytes + ins_bsize + pad_bsize);

    // Open the main gap: shifts `[pos, old_end + back_padding)` right by `ins_bsize` in a single
    // pass (`uninitialized_move` directly into final position).
    auto data_it =
      array().insert_any(array().begin() + (std::ptrdiff_t{padding_bytes} + offset), ins_bsize);

    // Open room for the trailing pad_end elements just before the back padding.
    // This only has to move `padding_bytes` worth of bytes, not the whole tail.
    if (pad_bsize > 0) {
      array().insert_any(array().begin() + (padding_bytes + old_bsize + ins_bsize), pad_bsize);
    }

    storage().size() += size;
    return iterator{data_it};
  }

  /** Copies `[first, last)` into the uninitialized elements starting at `pos`. */
  template<typename It>
  void copy_uninit(const_iterator pos, It first, It last) {
    const std::ptrdiff_t offset = pos.raw() - span().data();
    for (std::byte* dst = span().data() + offset; first != last; ++first, dst += element_bytes) {
      this->store(dst, *first);
    }
  }

  /** Inserts the elements of `[first, last)` before `pos`. */
  template<typename It>
  void insert(const_iterator pos, It first, It last) {
    const auto insize = *safe_cast<Size>(std::distance(first, last));
    copy_uninit(insert_any(pos, insize), first, last);
  }

private:
  using Base::byte_size;
  using Base::storage;

  /** Returns the underlying padded byte array. */
  [[nodiscard]] decltype(auto) array(this auto&& self) {
    return self.storage().array();
  }
  /** Returns the byte span of the stored elements, excluding padding. */
  [[nodiscard]] decltype(auto) span(this auto&& self) {
    return self.storage().span();
  }
};

/** An owning, growable array of packed `ByteInt::byte_num`-byte integers. */
template<typename ByteInt, std::size_t PaddingBytes, typename ByteAlloc = std::allocator<std::byte>>
using MultiByteIntegers = MultiByteIntegerArray<ByteInt, PaddingBytes, false, ByteAlloc>;
/**
 * An owning, growable array of `ValueOptional`-wrapped, packed `ByteInt::byte_num`-byte integers.
 */
template<typename ByteInt, std::size_t PaddingBytes, typename ByteAlloc = std::allocator<std::byte>>
using OptionalMultiByteIntegers = MultiByteIntegerArray<ByteInt, PaddingBytes, true, ByteAlloc>;
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_MULTI_BYTE_INTEGERS_HPP
