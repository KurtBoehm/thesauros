#ifndef INCLUDE_THESAUROS_CONTAINERS_MULTI_BYTE_INTEGERS_HPP
#define INCLUDE_THESAUROS_CONTAINERS_MULTI_BYTE_INTEGERS_HPP

#include <bit>
#include <cassert>
#include <compare>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <memory>
#include <span>
#include <type_traits>

#include "thesauros/containers/array/dynamic.hpp"
#include "thesauros/iterator/facades.hpp"
#include "thesauros/utility/arrow-proxy.hpp"
#include "thesauros/utility/inlining.hpp"

namespace thes {
template<typename TByteInt, std::size_t tPaddingBytes,
         template<typename> typename TAllocator = std::allocator>
struct MultiByteIntegers;

template<typename TByteInt, std::size_t tPaddingBytes,
         template<typename> typename TAllocator = std::allocator>
struct MultiByteSubRange {
  using Base = MultiByteIntegers<TByteInt, tPaddingBytes, TAllocator>;
  using Size = std::size_t;
  static constexpr std::size_t element_bytes = TByteInt::byte_num;

  MultiByteSubRange(const Base& base, Size begin, Size end)
      : base_(&base), begin_(begin), end_(end) {}

  const Base& base() const {
    return *base_;
  }

  [[nodiscard]] std::span<const std::byte> byte_span() const {
    const Size byte_begin = byte_size(begin_);
    const Size byte_end = byte_size(end_);
    return base_->byte_span().subspan(byte_begin, byte_end - byte_begin);
  }

private:
  static Size byte_size(Size size) THES_ALWAYS_INLINE {
    return size * element_bytes;
  }

  const Base* base_;
  Size begin_;
  Size end_;
};

template<typename TByteInt, std::size_t tPaddingBytes, template<typename> typename TAllocator>
struct MultiByteIntegers {
  static_assert(std::endian::native == std::endian::little ||
                  std::endian::native == std::endian::big,
                "Only big and little endian systems are supported!");

  using Value = TByteInt::Unsigned;
  using Size = std::size_t;

  using value_type = Value;
  using size_type = Size;

  static constexpr std::size_t element_bytes = TByteInt::byte_num;
  static constexpr std::size_t padding_bytes = tPaddingBytes;
  static constexpr Value int_bytes = sizeof(Value);

  using Allocator = TAllocator<std::byte>;
  using Data = DynamicArrayDefault<std::byte, Allocator>;

  static_assert(element_bytes <= int_bytes);
  static_assert(padding_bytes >= int_bytes);

  static constexpr std::size_t overhead_bits = TByteInt::overhead_bit_num;
  static constexpr Value mask = TByteInt::max;

  struct IntRef {
    explicit IntRef(std::byte* ptr) : ptr_(ptr) {}
    IntRef(const IntRef&) = delete;
    IntRef(IntRef&&) noexcept = default;
    ~IntRef() = default;

    IntRef& operator=(const IntRef& ref) {
      if (&ref != this) {
        store(ptr_, load(ref.ptr_));
      }
      return *this;
    }
    IntRef& operator=(IntRef&& ref) noexcept {
      store(ptr_, load(ref.ptr_));
      return *this;
    }
    IntRef& operator=(Value value) {
      store(ptr_, value);
      return *this;
    }

    operator Value() const {
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

    static void iadd(auto& self, Diff d) {
      self.ptr_ += byte_size(d);
    }
    static void isub(auto& self, Diff d) {
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
    static Diff byte_size(Diff d) {
      return d * Diff{element_bytes};
    }
  };

  template<bool tConst>
  struct Iterator : public IteratorFacade<Iterator<tConst>, IterProv<tConst>> {
    using Container = MultiByteIntegers;
    using Ptr = std::conditional_t<tConst, const std::byte, std::byte>*;
    friend IterProv<tConst>;

    explicit Iterator() = default;
    explicit Iterator(Ptr ptr) : ptr_{ptr} {}

    [[nodiscard]] Ptr raw() const {
      return ptr_;
    }

  private:
    Ptr ptr_{nullptr};
  };

  using ConstSubRange = MultiByteSubRange<TByteInt, tPaddingBytes, TAllocator>;

  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;

  static MultiByteIntegers all_set(Value size) {
    MultiByteIntegers mbi(size);
    std::uninitialized_fill_n(mbi.data_.data(), byte_size(mbi.size_),
                              std::byte{std::numeric_limits<unsigned char>::max()});
    return mbi;
  }

  MultiByteIntegers() : data_(int_bytes){};
  explicit MultiByteIntegers(Value size) : data_(effective_allocation(size)), size_(size) {}
  MultiByteIntegers(std::initializer_list<Value> init)
      : data_(effective_allocation(init.size())), size_(init.size()) {
    std::copy(init.begin(), init.end(), begin());
  };

  iterator begin() {
    return iterator(data_.data());
  }
  const_iterator begin() const {
    return const_iterator(data_.data());
  }
  iterator end() {
    return iterator(data_.data() + byte_size(size_));
  }
  const_iterator end() const {
    return const_iterator(data_.data() + byte_size(size_));
  }

  [[nodiscard]] Size size() const {
    return size_;
  }

  decltype(auto) operator[](Size i) const {
    return load(data_.data() + byte_size(i));
  }
  decltype(auto) operator[](Size i) {
    return IntRef{data_.data() + byte_size(i)};
  }

  decltype(auto) front() const {
    return load(data_.data());
  }
  decltype(auto) front() {
    return IntRef{data_.data()};
  }

  void push_back(Value value) {
    const Size size = byte_size(size_);
    assert(data_.size() == size + padding_bytes);

    data_.expand(data_.size() + element_bytes);
    ++size_;

    store_full(data_.begin() + size, value);
  }

  void pop_back() {
    assert(data_.size() == effective_allocation(size_));
    --size_;
    data_.shrink(data_.size() - element_bytes);
  }

  void reserve(Size allocation) {
    data_.reserve(effective_allocation(allocation));
  }

  [[nodiscard]] std::span<const std::byte> byte_span() const {
    return std::span{data_.begin(), byte_size(size_)};
  }
  [[nodiscard]] std::span<std::byte> byte_span() {
    return std::span{data_.begin(), byte_size(size_)};
  }

  ConstSubRange sub_range(Size begin, Size end) const {
    return ConstSubRange(*this, begin, end);
  }

private:
  static Size effective_allocation(Size allocation) THES_ALWAYS_INLINE {
    return byte_size(allocation) + padding_bytes;
  }
  static Size byte_size(Size size) THES_ALWAYS_INLINE {
    return size * element_bytes;
  }

  static Value load(const std::byte* ptr) THES_ALWAYS_INLINE {
    Value output;
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

  Data data_{};
  Size size_{0};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_MULTI_BYTE_INTEGERS_HPP
