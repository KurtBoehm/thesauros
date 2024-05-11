#ifndef INCLUDE_THESAUROS_CONTAINERS_DYNAMIC_BUFFER_HPP
#define INCLUDE_THESAUROS_CONTAINERS_DYNAMIC_BUFFER_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <span>
#include <string_view>

namespace thes {
struct DynamicBuffer {
  DynamicBuffer() = default;
  explicit DynamicBuffer(std::size_t size)
      : begin_(static_cast<std::byte*>(std::malloc(size))), size_(size) {}
  DynamicBuffer(const DynamicBuffer&) = delete;
  DynamicBuffer(DynamicBuffer&& other) noexcept : begin_(other.begin_), size_(other.size_) {
    other.begin_ = nullptr;
    other.size_ = 0;
  }
  DynamicBuffer& operator=(const DynamicBuffer&) = delete;
  DynamicBuffer& operator=(DynamicBuffer&&) = delete;
  ~DynamicBuffer() {
    std::free(begin_);
  }

  void resize(std::size_t new_size) {
    if (new_size == size_) {
      return;
    }
    begin_ = static_cast<std::byte*>(std::realloc(begin_, new_size));
    size_ = new_size;
  }

  [[nodiscard]] std::byte* data() {
    return begin_;
  }
  [[nodiscard]] const std::byte* data() const {
    return begin_;
  }

  [[nodiscard]] std::uint8_t* data_u8() {
    return reinterpret_cast<std::uint8_t*>(begin_);
  }
  [[nodiscard]] const std::uint8_t* data_u8() const {
    return reinterpret_cast<std::uint8_t*>(begin_);
  }

  [[nodiscard]] char* data_char() {
    return reinterpret_cast<char*>(begin_);
  }
  [[nodiscard]] const char* data_char() const {
    return reinterpret_cast<char*>(begin_);
  }

  [[nodiscard]] std::byte operator[](std::size_t index) const {
    return begin_[index];
  }
  [[nodiscard]] std::byte& operator[](std::size_t index) {
    return begin_[index];
  }

  [[nodiscard]] std::size_t size() const {
    return size_;
  }

  [[nodiscard]] std::span<std::byte> span() {
    return {begin_, size_};
  }
  [[nodiscard]] std::span<const std::byte> span() const {
    return {begin_, size_};
  }

private:
  std::byte* begin_{};
  std::size_t size_{};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_DYNAMIC_BUFFER_HPP
