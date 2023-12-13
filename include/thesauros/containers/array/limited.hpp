#ifndef INCLUDE_THESAUROS_CONTAINERS_ARRAY_LIMITED_HPP
#define INCLUDE_THESAUROS_CONTAINERS_ARRAY_LIMITED_HPP

#include <array>
#include <cassert>
#include <cstddef>

namespace thes {
// Access to element beyond the size (up to the capacity) is allowed, but discouraged
template<typename T, std::size_t tCapacity>
struct LimitedArray {
  using Value = T;
  static constexpr std::size_t capacity = tCapacity;

  using Array = std::array<Value, capacity>;
  using const_iterator = const T*;

  explicit constexpr LimitedArray(std::size_t size) : size_(size) {
    assert(size_ <= capacity);
  }
  template<typename... Ts>
  requires(sizeof...(Ts) <= tCapacity)
  explicit constexpr LimitedArray(Ts... values) : size_(sizeof...(Ts)), data_{values...} {
    assert(size_ <= capacity);
  }

  template<typename TIt>
  LimitedArray(TIt first, TIt last) : size_(std::distance(first, last)) {
    assert(size_ <= capacity);

    auto out_it = data_.begin();
    for (auto in_it = first; in_it != last; ++in_it) {
      *(out_it++) = *in_it;
    }
  }

  constexpr const T& operator[](std::size_t idx) const {
    assert(idx < capacity);
    return data_[idx];
  }

  [[nodiscard]] constexpr const T* begin() const {
    return data_.begin();
  }
  [[nodiscard]] constexpr const T* end() const {
    return data_.begin() + size_;
  }

  [[nodiscard]] constexpr const T* data() const {
    return data_.data();
  }
  [[nodiscard]] constexpr T* data() {
    return data_.data();
  }

  [[nodiscard]] constexpr const Array& as_array() const {
    return data_;
  }

  constexpr friend bool operator==(const LimitedArray& a1, const LimitedArray& a2) {
    if (a1.size_ != a2.size_) {
      return false;
    }
    for (std::size_t i = 0; i < a1.size_; ++i) {
      if (a1[i] != a2[i]) {
        return false;
      }
    }
    return true;
  }

private:
  std::size_t size_;
  Array data_{};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_ARRAY_LIMITED_HPP
