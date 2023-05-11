#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_CAPACITY_STRING_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_CAPACITY_STRING_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <ostream>
#include <string_view>

namespace thes {
template<std::size_t tCapacity>
struct StaticCapacityString {
  using value_type = char;

  template<std::size_t tSize>
  requires(tSize <= tCapacity)
  constexpr explicit StaticCapacityString(char ptr[tSize]) : size_{tSize} {
    std::copy(ptr, ptr + tSize, data_.data());
  }
  StaticCapacityString() = default;

  [[nodiscard]] std::string_view view() const {
    return {data_.data(), size_};
  }

  [[nodiscard]] const char* data() const {
    return data_.data();
  }
  char* data() {
    return data_.data();
  }

  [[nodiscard]] const char* begin() const {
    return data_.data();
  }
  [[nodiscard]] char* begin() {
    return data_.data();
  }

  [[nodiscard]] const char* end() const {
    return data_.data() + size_;
  }
  [[nodiscard]] char* end() {
    return data_.data() + size_;
  }

  [[nodiscard]] std::size_t size() const {
    return size_;
  }
  std::size_t& size() {
    return size_;
  }

  void push_back(char c) {
    assert(size_ < tCapacity);
    data_[size_++] = c;
  }

  friend std::ostream& operator<<(std::ostream& s, const StaticCapacityString& str) {
    return s << str.view();
  }

private:
  std::array<char, tCapacity> data_{};
  std::size_t size_{0};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_CAPACITY_STRING_HPP
