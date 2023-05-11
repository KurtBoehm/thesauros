#ifndef INCLUDE_THESAUROS_CONTAINERS_MULTI_BYTE_INTEGERS_HPP
#define INCLUDE_THESAUROS_CONTAINERS_MULTI_BYTE_INTEGERS_HPP

#include "thesauros/containers/array/dynamic.hpp"
#include "thesauros/iterator/facades.hpp"
#include "thesauros/utility/arrow-proxy.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>

namespace thes {
template<typename TByteInt, std::size_t tPaddingBytes,
         template<typename> typename TAllocator = std::allocator>
struct MultiByteIntegers {
  static_assert(std::endian::native == std::endian::little ||
                  std::endian::native == std::endian::big,
                "Only big and little endian systems are supported!");

  using size_type = std::size_t;
  using value_type = typename TByteInt::Type;

  static constexpr std::size_t element_bytes = TByteInt::byte_num;
  static constexpr std::size_t padding_bytes = tPaddingBytes;
  static constexpr value_type int_bytes = sizeof(value_type);

  using Byte = std::uint8_t;
  using Allocator = TAllocator<Byte>;
  using Data = DynamicArrayDefault<Byte, Allocator>;

  static_assert(element_bytes <= int_bytes);
  static_assert(padding_bytes >= int_bytes);

  static constexpr std::size_t overhead_bits = TByteInt::overhead_bit_num;
  static constexpr value_type mask = TByteInt::max;

  struct IntRef {
    explicit IntRef(Byte* ptr) : ptr_(ptr) {}
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
    IntRef& operator=(value_type value) {
      store(ptr_, value);
      return *this;
    }

    operator value_type() const {
      return load(ptr_);
    }

    friend void swap(IntRef vw1, IntRef vw2) {
      value_type v1 = vw1;
      value_type v2 = vw2;
      vw1 = v2;
      vw2 = v1;
    }

    friend void swap(IntRef vw1, value_type& i2) {
      value_type v1 = vw1;
      vw1 = i2;
      i2 = v1;
    }
    friend void swap(value_type& i1, IntRef vw2) {
      value_type v2 = vw2;
      vw2 = i1;
      i1 = v2;
    }

  private:
    Byte* ptr_;
  };

  template<bool tConst>
  struct IterProv {
    using Ref = std::conditional_t<tConst, value_type, IntRef>;
    using Ptr = ArrowProxy<value_type>;
    using Diff = std::ptrdiff_t;

    struct IterTypes {
      using IterValue = value_type;
      using IterRef = Ref;
      using IterPtr = Ptr;
      using IterDiff = Diff;
    };

    static Ref deref(const auto& self) {
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
    using Ptr = std::conditional_t<tConst, const Byte, Byte>*;
    friend IterProv<tConst>;

    explicit Iterator() = default;
    explicit Iterator(Ptr ptr) : ptr_{ptr} {}

    [[nodiscard]] Ptr raw() const {
      return ptr_;
    }

  private:
    Ptr ptr_{nullptr};
  };

  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;

  static MultiByteIntegers all_set(value_type size) {
    MultiByteIntegers mbi(size);
    std::uninitialized_fill_n(mbi.data_.data(), byte_size(mbi.size_),
                              std::numeric_limits<Byte>::max());
    return mbi;
  }

  MultiByteIntegers() : data_(int_bytes){};
  explicit MultiByteIntegers(value_type size) : data_(effective_allocation(size)), size_(size) {}
  MultiByteIntegers(std::initializer_list<value_type> init)
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

  [[nodiscard]] size_type size() const {
    return size_;
  }

  decltype(auto) operator[](size_type i) const {
    return load(data_.data() + byte_size(i));
  }
  decltype(auto) operator[](size_type i) {
    return IntRef{data_.data() + byte_size(i)};
  }

  void push_back(value_type value) {
    const size_type size = byte_size(size_);
    assert(data_.size() == size + int_bytes);

    data_.expand(data_.size() + element_bytes);
    ++size_;

    store_full(data_.begin() + size, value);
  }

  void pop_back() {
    assert(data_.size() == effective_allocation(size_));
    --size_;
    data_.shrink(data_.size() - element_bytes);
  }

  void reserve(size_type allocation) {
    data_.reserve(effective_allocation(allocation));
  }

private:
  static size_type effective_allocation(size_type allocation) {
    return byte_size(allocation) + padding_bytes;
  }
  static size_type byte_size(size_type size) {
    return size * element_bytes;
  }

  static value_type load(const Byte* ptr) {
    value_type output;
    std::memcpy(&output, ptr, int_bytes);
    if constexpr (std::endian::native == std::endian::little) {
      return output & mask;
    }
    if constexpr (std::endian::native == std::endian::big) {
      return output >> overhead_bits;
    }
    return output;
  }

  static value_type& store_transform(value_type& value) noexcept {
    if constexpr (std::endian::native == std::endian::big) {
      value <<= overhead_bits;
    }
    return value;
  }
  static void store(Byte* ptr, value_type value) noexcept {
    std::memcpy(ptr, &store_transform(value), element_bytes);
  }
  static void store_full(Byte* ptr, value_type value) {
    std::memcpy(ptr, &store_transform(value), int_bytes);
  }

  Data data_{};
  size_type size_{0};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_MULTI_BYTE_INTEGERS_HPP
