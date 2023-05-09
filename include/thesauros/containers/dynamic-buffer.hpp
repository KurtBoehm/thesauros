#ifndef INCLUDE_THESAUROS_CONTAINERS_DYNAMIC_BUFFER_HPP
#define INCLUDE_THESAUROS_CONTAINERS_DYNAMIC_BUFFER_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <span>

namespace thes {
struct DynamicBuffer {
  DynamicBuffer() = default;
  explicit DynamicBuffer(std::size_t size)
      : begin_(static_cast<std::uint8_t*>(std::malloc(size))), size_(size) {}
  DynamicBuffer(const DynamicBuffer&) = delete;
  DynamicBuffer(DynamicBuffer&&) = delete;
  DynamicBuffer& operator=(const DynamicBuffer&) = delete;
  DynamicBuffer& operator=(DynamicBuffer&&) = delete;
  ~DynamicBuffer() {
    std::free(begin_);
  }

  void resize(std::size_t new_size) {
    if (new_size == size_) {
      return;
    }
    begin_ = static_cast<std::uint8_t*>(std::realloc(begin_, new_size));
    size_ = new_size;
  }

  [[nodiscard]] std::uint8_t* data() {
    return begin_;
  }

  [[nodiscard]] std::uint8_t operator[](std::size_t index) const {
    return begin_[index];
  }
  [[nodiscard]] std::uint8_t& operator[](std::size_t index) {
    return begin_[index];
  }

  [[nodiscard]] std::size_t size() const {
    return size_;
  }

  [[nodiscard]] std::span<std::uint8_t> span() {
    return {begin_, size_};
  }
  [[nodiscard]] std::span<const std::uint8_t> span() const {
    return {begin_, size_};
  }

private:
  std::uint8_t* begin_{};
  std::size_t size_{};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_DYNAMIC_BUFFER_HPP
