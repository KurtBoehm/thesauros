// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STRING_STATIC_CAPACITY_STRING_HPP
#define INCLUDE_THESAUROS_STRING_STATIC_CAPACITY_STRING_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <string_view>

namespace thes {
/**
 * A fixed-capacity, stack-allocated, always null-terminated string with a `std::string`-like
 * interface.
 */
template<std::size_t Capacity>
struct StaticCapacityString {
  using value_type = char;
  using size_type = std::size_t;
  using reference = char&;
  using const_reference = const char&;
  using iterator = char*;
  using const_iterator = const char*;

  static constexpr size_type capacity = Capacity;

  //------------------------------------------------------------------------------------------------
  // Construction
  //------------------------------------------------------------------------------------------------

  constexpr StaticCapacityString() noexcept = default;

  /** Constructs a string containing the characters of `text`, excluding the null terminator. */
  template<std::size_t Size>
  requires(Size - 1 <= Capacity)
  constexpr StaticCapacityString(const char (&text)[Size]) noexcept : size_{Size - 1} { // NOLINT
    std::copy(text, text + (Size - 1), data_.data());
  }

  /** Constructs a string of `count` copies of `c`; `count` must be at most `capacity`. */
  constexpr StaticCapacityString(size_type count, char c) noexcept : size_{count} {
    assert(count <= Capacity);
    std::fill(data_.data(), data_.data() + count, c);
  }

  /**
   * Constructs a string containing the characters of `text`; `text.size()` must be at most
   * `capacity`.
   */
  explicit constexpr StaticCapacityString(std::string_view text) noexcept : size_{text.size()} {
    assert(text.size() <= Capacity);
    std::copy(text.begin(), text.end(), data_.data());
  }

  //------------------------------------------------------------------------------------------------
  // Element access
  //------------------------------------------------------------------------------------------------

  /** Implicitly converts to a view of the string’s current contents. */
  [[nodiscard]] constexpr operator std::string_view() const noexcept { // NOLINT
    return {data_.data(), size_};
  }
  [[nodiscard]] constexpr std::string_view view() const noexcept {
    return {data_.data(), size_};
  }

  [[nodiscard]] constexpr auto data(this auto& self) noexcept {
    return self.data_.data();
  }

  /** A null-terminated view of the string’s contents, suitable for C APIs. */
  [[nodiscard]] constexpr const char* c_str() const noexcept {
    return data_.data();
  }

  [[nodiscard]] constexpr auto& operator[](this auto& self, size_type idx) noexcept {
    assert(idx <= self.size_);
    return self.data_[idx];
  }

  /** Bounds-checked element access; throws `std::out_of_range` if `idx >= size()`. */
  [[nodiscard]] constexpr auto& at(this auto& self, size_type idx) {
    if (idx >= self.size_) {
      throw std::out_of_range{"StaticCapacityString::at: index out of range"};
    }
    return self.data_[idx];
  }

  [[nodiscard]] constexpr auto& front(this auto& self) noexcept {
    assert(!self.empty());
    return self.data_[0];
  }
  [[nodiscard]] constexpr auto& back(this auto& self) noexcept {
    assert(!self.empty());
    return self.data_[self.size_ - 1];
  }

  //------------------------------------------------------------------------------------------------
  // Iterators
  //------------------------------------------------------------------------------------------------

  [[nodiscard]] constexpr auto begin(this auto& self) noexcept {
    return self.data_.data();
  }
  [[nodiscard]] constexpr auto end(this auto& self) noexcept {
    return self.data_.data() + self.size_;
  }
  [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
    return begin();
  }
  [[nodiscard]] constexpr const_iterator cend() const noexcept {
    return end();
  }

  //------------------------------------------------------------------------------------------------
  // Capacity
  //------------------------------------------------------------------------------------------------

  [[nodiscard]] constexpr size_type size() const noexcept {
    return size_;
  }
  [[nodiscard]] constexpr size_type length() const noexcept {
    return size_;
  }
  [[nodiscard]] constexpr bool empty() const noexcept {
    return size_ == 0;
  }

  //------------------------------------------------------------------------------------------------
  // Modifiers
  //------------------------------------------------------------------------------------------------

  constexpr void clear() noexcept {
    size_ = 0;
    terminate();
  }

  /** Sets the length to `new_size`, filling any newly added characters with `c`. */
  constexpr void resize(size_type new_size, char c = '\0') noexcept {
    assert(new_size <= Capacity);
    if (new_size > size_) {
      std::fill(data_.data() + size_, data_.data() + new_size, c);
    }
    size_ = new_size;
    terminate();
  }

  /**
   * Sets the length to `new_size` without touching the buffer’s contents beyond the new terminator;
   * intended for callers that have already written directly into `data()`, e.g. via
   * `std::to_chars`.
   */
  constexpr void set_size(size_type new_size) noexcept {
    assert(new_size <= Capacity);
    size_ = new_size;
    terminate();
  }

  constexpr void push_back(char c) noexcept {
    assert(size_ < Capacity);
    data_[size_++] = c;
    terminate();
  }
  constexpr void pop_back() noexcept {
    assert(!empty());
    --size_;
    terminate();
  }

  constexpr StaticCapacityString& append(std::string_view text) noexcept {
    assert(size_ + text.size() <= Capacity);
    std::copy(text.begin(), text.end(), data_.data() + size_);
    size_ += text.size();
    terminate();
    return *this;
  }
  constexpr StaticCapacityString& operator+=(char c) noexcept {
    push_back(c);
    return *this;
  }
  constexpr StaticCapacityString& operator+=(std::string_view text) noexcept {
    return append(text);
  }

  //------------------------------------------------------------------------------------------------
  // Comparison
  //------------------------------------------------------------------------------------------------

  [[nodiscard]] friend constexpr bool operator==(const StaticCapacityString& self,
                                                 std::string_view other) noexcept {
    return self.view() == other;
  }
  [[nodiscard]] friend constexpr auto operator<=>(const StaticCapacityString& self,
                                                  std::string_view other) noexcept {
    return self.view() <=> other;
  }

  friend constexpr std::string_view format_as(const StaticCapacityString& self) {
    return self.view();
  }

private:
  /** Writes the null terminator at `data_[size_]`, keeping the buffer C-string-safe. */
  constexpr void terminate() noexcept {
    data_[size_] = '\0';
  }

  std::array<char, Capacity + 1> data_{};
  size_type size_{0};
};

template<std::size_t Size>
StaticCapacityString(const char (&)[Size]) -> StaticCapacityString<Size - 1>;
} // namespace thes

#endif // INCLUDE_THESAUROS_STRING_STATIC_CAPACITY_STRING_HPP
