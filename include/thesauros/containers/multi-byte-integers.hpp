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
#include "thesauros/iterator/facades.hpp"
#include "thesauros/iterator/provider-reverse.hpp"
#include "thesauros/utility/arrow-proxy.hpp"
#include "thesauros/utility/inlining.hpp"
#include "thesauros/utility/integral-value.hpp"
#include "thesauros/utility/type-tag.hpp"
#include "thesauros/utility/type-transformations.hpp"
#include "thesauros/utility/value-optional.hpp"

namespace thes {
namespace impl {
template<typename TByteInt, std::size_t tPaddingBytes, typename TByteAlloc>
struct ArrayStorage {
  using Size = std::size_t;

  static constexpr std::size_t padding_bytes = tPaddingBytes;
  static constexpr std::size_t element_bytes = TByteInt::byte_num;

  using Data = DynamicArray<std::byte, DefaultInit, DoublingGrowth, TByteAlloc>;

  ArrayStorage() : data_(padding_bytes) {};
  explicit ArrayStorage(std::size_t size) : data_(effective_allocation(size)), size_(size) {}

  [[nodiscard]] Data& data() {
    return data_;
  }
  [[nodiscard]] const Data& data() const {
    return data_;
  }

  [[nodiscard]] Size& size() {
    return size_;
  }
  [[nodiscard]] Size size() const {
    return size_;
  }

  static Size effective_allocation(Size allocation) THES_ALWAYS_INLINE {
    return allocation * element_bytes + padding_bytes;
  }

private:
  Data data_{};
  Size size_{0};
};

template<bool tIsConst, typename TByteInt>
struct ViewStorage {
  using CByte = ConditionalConst<tIsConst, std::byte>;
  using Size = std::size_t;
  static constexpr std::size_t element_bytes = TByteInt::byte_num;

  ViewStorage(CByte* data, Size size) : data_(data), size_(size) {}

  [[nodiscard]] std::span<std::byte> data()
  requires(!tIsConst)
  {
    return {data_, size_ * element_bytes};
  }
  [[nodiscard]] std::span<const std::byte> data() const {
    return {data_, size_ * element_bytes};
  }

  [[nodiscard]] Size size() const {
    return size_;
  }

private:
  CByte* data_{};
  Size size_{0};
};
} // namespace impl

template<bool tIsConst, typename TByteInt, std::size_t tPaddingBytes, bool tOptional>
struct MultiByteSubRange;

template<typename TDerived, typename TByteInt, std::size_t tPaddingBytes, bool tOptional,
         typename TStorage>
struct MultiByteIntegersBase {
  static_assert(std::endian::native == std::endian::little ||
                  std::endian::native == std::endian::big,
                "Only big and little endian systems are supported!");

  using BaseValue = TByteInt::Unsigned;
  static constexpr BaseValue int_bytes = sizeof(BaseValue);
  static constexpr BaseValue mask = TByteInt::max;
  static constexpr bool is_mutable =
    std::is_const_v<std::remove_pointer_t<decltype(std::declval<TStorage>().data().data())>>;

  using Value = std::conditional_t<tOptional, ValueOptional<BaseValue, mask>, BaseValue>;
  using Size = std::size_t;

  using value_type = Value;
  using size_type = Size;

  static constexpr std::size_t padding_bytes = tPaddingBytes;
  static constexpr std::size_t element_bytes = TByteInt::byte_num;
  static constexpr std::size_t overhead_bits = TByteInt::overhead_bit_num;

  using Storage = std::decay_t<TStorage>;

  static_assert(element_bytes <= int_bytes);
  static_assert(padding_bytes >= int_bytes);

  struct IntRef {
    explicit IntRef(std::byte* ptr) : ptr_(ptr) {}
    IntRef(const IntRef&) = delete;
    IntRef(IntRef&&) noexcept = default;
    ~IntRef() = default;

    IntRef& operator=(const IntRef& ref) { // NOLINT
      store(ptr_, load(ref.ptr_));
      return *this;
    }
    IntRef& operator=(IntRef&& ref) noexcept {
      store(ptr_, load(ref.ptr_));
      return *this;
    }
    IntRef& operator=(Value value) {
      if constexpr (!tOptional) {
        assert(value == (value & mask));
      }
      store(ptr_, value);
      return *this;
    }

    operator Value() const { // NOLINT
      return load(ptr_);
    }

    friend void swap(IntRef vw1, IntRef vw2) noexcept {
      Value v1 = vw1;
      Value v2 = vw2;
      vw1 = v2;
      vw2 = v1;
    }

    friend void swap(IntRef vw1, Value& i2) noexcept {
      Value v1 = vw1;
      vw1 = i2;
      i2 = v1;
    }
    friend void swap(Value& i1, IntRef vw2) noexcept {
      Value v2 = vw2;
      vw2 = i1;
      i1 = v2;
    }

  private:
    std::byte* ptr_;
  };

  template<bool tConst>
  struct IterProv {
    using Ref = std::conditional_t<tConst, Value, IntRef>;
    using Ptr = ArrowProxy<Value>;
    using Diff = std::ptrdiff_t;

    struct IterTypes {
      using IterValue = Value;
      using IterRef = Ref;
      using IterPtr = Ptr;
      using IterDiff = Diff;
    };

    static Ref deref(const auto& self) {
      assert(self.ptr_ != nullptr);
      if constexpr (tConst) {
        return load(self.ptr_);
      } else {
        return Ref{self.ptr_};
      }
    }

    static void incr(auto& self) {
      self.ptr_ += element_bytes;
    }
    static void decr(auto& self) {
      self.ptr_ -= element_bytes;
    }

    static void iadd(auto& self, auto d) {
      self.ptr_ += byte_size(d);
    }
    static void isub(auto& self, auto d) {
      self.ptr_ -= byte_size(d);
    }

    static bool eq(const auto& i1, const auto& i2) {
      return i1.ptr_ == i2.ptr_;
    }
    static std::strong_ordering three_way(const auto& i1, const auto& i2) {
      return i1.ptr_ <=> i2.ptr_;
    }

    static Diff sub(const auto& i1, const auto& i2) {
      static constexpr Diff eb = element_bytes;
      Diff diff = i1.ptr_ - i2.ptr_;
      assert(diff % eb == 0);
      return diff / eb;
    }

  private:
    template<typename T>
    static auto byte_size(T d) {
      using Integral = IntegralValue<T>;
      using Ret = std::conditional_t<std::unsigned_integral<Integral>, Size, Diff>;
      return Ret{d} * Ret{element_bytes};
    }
  };

  template<bool tConst, bool tReverse>
  struct BaseIterator
      : public IteratorFacade<
          BaseIterator<tConst, tReverse>,
          std::conditional_t<
            tReverse, iter_provider::Reverse<IterProv<tConst>, BaseIterator<tConst, tReverse>>,
            IterProv<tConst>>> {
    using Container = TDerived;
    using Ptr = std::conditional_t<tConst, const std::byte, std::byte>*;
    friend IterProv<tConst>;

    explicit BaseIterator() = default;
    explicit BaseIterator(Ptr ptr) : ptr_{ptr} {}

    [[nodiscard]] Ptr raw() const {
      return ptr_;
    }
    operator BaseIterator<true, tReverse>() const { // NOLINT
      return BaseIterator<true, tReverse>{ptr_};
    }

  private:
    Ptr ptr_{nullptr};
  };

  using ConstSubRange = MultiByteSubRange<true, TByteInt, tPaddingBytes, tOptional>;
  using MutableSubRange = MultiByteSubRange<false, TByteInt, tPaddingBytes, tOptional>;

  using iterator = BaseIterator<false, false>;
  using const_iterator = BaseIterator<true, false>;
  using reverse_iterator = BaseIterator<false, true>;
  using const_reverse_iterator = BaseIterator<true, true>;

  explicit MultiByteIntegersBase(TStorage&& storage) : storage_(std::forward<TStorage>(storage)) {};

  iterator begin() {
    return iterator(data().data());
  }
  const_iterator begin() const {
    return const_iterator(data().data());
  }
  iterator end() {
    return iterator(data().data() + byte_size(storage_.size()));
  }
  const_iterator end() const {
    return const_iterator(data().data() + byte_size(storage_.size()));
  }

  reverse_iterator rbegin() {
    return reverse_iterator(data().data() + byte_size(storage_.size()));
  }
  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(data().data() + byte_size(storage_.size()));
  }
  reverse_iterator rend() {
    return reverse_iterator(data().data());
  }
  const_reverse_iterator rend() const {
    return const_reverse_iterator(data().data());
  }

  [[nodiscard]] Size size() const {
    return storage_.size();
  }

  decltype(auto) operator[](Size i) const {
    assert(i < size());
    return load(data().data() + byte_size(i));
  }
  decltype(auto) operator[](Size i)
  requires(!is_mutable)
  {
    return IntRef{data().data() + byte_size(i)};
  }

  decltype(auto) front() const {
    return load(data().data());
  }
  decltype(auto) front() {
    return IntRef{data().data()};
  }

  decltype(auto) back() const {
    assert(storage_.size() > 0);
    return load(data().data() + byte_size(storage_.size() - 1));
  }
  decltype(auto) back() {
    assert(storage_.size() > 0);
    return IntRef{data().data() + byte_size(storage_.size() - 1)};
  }

  [[nodiscard]] std::span<const std::byte> byte_span() const {
    return std::span{data().begin(), byte_size(storage_.size())};
  }
  [[nodiscard]] std::span<std::byte> byte_span() {
    return std::span{data().begin(), byte_size(storage_.size())};
  }

  ConstSubRange sub_range(Size begin, Size end) const {
    assert(end >= begin);
    return ConstSubRange(data().data() + byte_size(begin), end - begin);
  }
  MutableSubRange sub_range(Size begin, Size end) {
    assert(end >= begin);
    return MutableSubRange(data().data() + byte_size(begin), end - begin);
  }

  ConstSubRange full_sub_range() const {
    return sub_range(0, size());
  }
  MutableSubRange full_sub_range() {
    return sub_range(0, size());
  }

  void to_file(FileWriter& writer) const {
    const Size stored_size = storage_.size();
    writer.write(std::span{&stored_size, 1});
    writer.write(byte_span());
  }

protected:
  static Size byte_size(Size size) THES_ALWAYS_INLINE {
    return size * element_bytes;
  }

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

  static Value& store_transform(Value& value) noexcept THES_ALWAYS_INLINE {
    if constexpr (std::endian::native == std::endian::big) {
      value <<= overhead_bits;
    }
    return value;
  }
  static void store(std::byte* ptr, Value value) noexcept THES_ALWAYS_INLINE {
    std::memcpy(ptr, &store_transform(value), element_bytes);
  }
  static void store_full(std::byte* ptr, Value value) THES_ALWAYS_INLINE {
    std::memcpy(ptr, &store_transform(value), int_bytes);
  }

  Storage& storage() {
    return storage_;
  }
  const Storage& storage() const {
    return storage_;
  }

  [[nodiscard]] decltype(auto) data() const {
    return storage_.data();
  }
  [[nodiscard]] decltype(auto) data() {
    return storage_.data();
  }

private:
  TStorage storage_;
};

template<bool tIsConst, typename TByteInt, std::size_t tPaddingBytes, bool tOptional>
struct MultiByteSubRange
    : public MultiByteIntegersBase<MultiByteSubRange<tIsConst, TByteInt, tPaddingBytes, tOptional>,
                                   TByteInt, tPaddingBytes, tOptional,
                                   impl::ViewStorage<tIsConst, TByteInt>> {
  using Storage = impl::ViewStorage<tIsConst, TByteInt>;
  using Base =
    MultiByteIntegersBase<MultiByteSubRange, TByteInt, tPaddingBytes, tOptional, Storage>;

  using Size = Base::Size;
  using CByte = Storage::CByte;

  MultiByteSubRange(CByte* data, Size size) : Base(Storage{data, size}) {}
};

template<typename TByteInt, std::size_t tPaddingBytes, bool tOptional, typename TByteAlloc>
struct MultiByteIntegerArray
    : public MultiByteIntegersBase<
        MultiByteIntegerArray<TByteInt, tPaddingBytes, tOptional, TByteAlloc>, TByteInt,
        tPaddingBytes, tOptional, impl::ArrayStorage<TByteInt, tPaddingBytes, TByteAlloc>> {
  using Storage = impl::ArrayStorage<TByteInt, tPaddingBytes, TByteAlloc>;
  using Base =
    MultiByteIntegersBase<MultiByteIntegerArray, TByteInt, tPaddingBytes, tOptional, Storage>;

  using Size = Base::Size;
  using Value = Base::Value;
  using iterator = Base::iterator;
  using const_iterator = Base::const_iterator;
  using Base::element_bytes;
  using Base::padding_bytes;

  static MultiByteIntegerArray from_file(FileReader& reader) {
    MultiByteIntegerArray out(reader.read(type_tag<Size>));
    reader.read(out.byte_span());
    return out;
  }

  static MultiByteIntegerArray create_all_set(std::size_t size)
  requires(!tOptional)
  {
    MultiByteIntegerArray mbi(size);
    std::fill_n(mbi.data().data(), byte_size(mbi.size()),
                std::byte{std::numeric_limits<unsigned char>::max()});
    return mbi;
  }
  static MultiByteIntegerArray create_empty(std::size_t size)
  requires(tOptional)
  {
    MultiByteIntegerArray mbi(size);
    std::fill_n(mbi.data().data(), byte_size(mbi.size()),
                std::byte{std::numeric_limits<unsigned char>::max()});
    return mbi;
  }

  MultiByteIntegerArray() : Base(Storage{}) {};
  explicit MultiByteIntegerArray(std::size_t size) : Base(Storage{size}) {}
  MultiByteIntegerArray(std::initializer_list<Value> init) : Base(Storage{init.size()}) {
    std::copy(init.begin(), init.end(), this->begin());
  };

  void push_back(Value value) {
    const Size size = byte_size(storage().size());
    assert(data().size() == size + padding_bytes);

    data().expand(data().size() + element_bytes);
    ++storage().size();

    this->store_full(data().begin() + size, value);
  }

  void pop_back() {
    assert(data().size() == Storage::effective_allocation(storage().size()));
    --storage().size();
    data().shrink(data().size() - element_bytes);
  }

  void reserve(Size allocation) {
    data().reserve(Storage::effective_allocation(allocation));
  }

  void set_all(Size first, Size last) {
    std::byte* ptr = data().data();
    std::fill(ptr + byte_size(first), ptr + byte_size(last),
              std::byte{std::numeric_limits<unsigned char>::max()});
  }

  iterator insert_uninit(const_iterator pos, Size size) {
    const Size old_bsize = byte_size(storage().size());
    assert(data().size() == old_bsize + padding_bytes);

    const Size new_bsize = old_bsize + byte_size(size);
    const std::ptrdiff_t offset = pos.raw() - data().data();

    data().expand(new_bsize + padding_bytes);
    storage().size() += size;

    std::byte* new_begin = data().data();
    std::byte* dst = new_begin + offset;
    std::move_backward(dst, new_begin + old_bsize, new_begin + new_bsize);

    return iterator{dst};
  }

  template<typename TIt>
  void copy_uninit(const_iterator pos, TIt first, TIt last) {
    const std::ptrdiff_t offset = pos.raw() - data().data();
    for (std::byte* dst = data().data() + offset; first != last; ++first, dst += element_bytes) {
      this->store(dst, *first);
    }
  }

  template<typename TIt>
  void insert(const_iterator pos, TIt first, TIt last) {
    const auto insize = *safe_cast<Size>(std::distance(first, last));
    copy_uninit(insert_uninit(pos, insize), first, last);
  }

private:
  using Base::byte_size;
  using Base::storage;

  [[nodiscard]] decltype(auto) data() const {
    return storage().data();
  }
  [[nodiscard]] decltype(auto) data() {
    return storage().data();
  }
};

template<typename TByteInt, std::size_t tPaddingBytes,
         typename TByteAlloc = std::allocator<std::byte>>
using MultiByteIntegers = MultiByteIntegerArray<TByteInt, tPaddingBytes, false, TByteAlloc>;
template<typename TByteInt, std::size_t tPaddingBytes,
         typename TByteAlloc = std::allocator<std::byte>>
using OptionalMultiByteIntegers = MultiByteIntegerArray<TByteInt, tPaddingBytes, true, TByteAlloc>;
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_MULTI_BYTE_INTEGERS_HPP
