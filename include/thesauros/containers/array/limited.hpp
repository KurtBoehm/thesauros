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

  explicit LimitedArray(std::size_t size) : size_(size) {
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

  const T& operator[](std::size_t idx) const {
    assert(idx < capacity);
    return data_[idx];
  }

  [[nodiscard]] const T* begin() const {
    return data_.begin();
  }
  [[nodiscard]] const T* end() const {
    return data_.begin() + size_;
  }

  [[nodiscard]] const T* data() const {
    return data_.data();
  }
  [[nodiscard]] T* data() {
    return data_.data();
  }

  [[nodiscard]] const Array& as_array() const {
    return data_;
  }

private:
  std::size_t size_;
  Array data_{};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_ARRAY_LIMITED_HPP
