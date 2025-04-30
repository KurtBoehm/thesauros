// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_DYNAMIC_BUFFER_HPP
#define INCLUDE_THESAUROS_CONTAINERS_DYNAMIC_BUFFER_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <span>

namespace thes {
struct DynamicBuffer {
  DynamicBuffer() = default;
  explicit DynamicBuffer(std::size_t size)
      : begin_(static_cast<std::byte*>(std::malloc(size))), alloc_(size), size_(size) {}
  DynamicBuffer(const DynamicBuffer&) = delete;
  DynamicBuffer(DynamicBuffer&& other) noexcept
      : begin_(other.begin_), alloc_(other.alloc_), size_(other.size_) {
    other.begin_ = nullptr;
    other.alloc_ = 0;
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
    if (new_size <= alloc_) {
      size_ = new_size;
      return;
    }
    auto* new_begin = static_cast<std::byte*>(std::realloc(begin_, new_size));
    if (new_begin == nullptr) {
      throw std::bad_alloc{};
    }
    begin_ = new_begin;
    alloc_ = new_size;
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
  std::size_t alloc_{};
  std::size_t size_{};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_DYNAMIC_BUFFER_HPP
