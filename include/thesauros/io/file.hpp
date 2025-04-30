// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_IO_FILE_HPP
#define INCLUDE_THESAUROS_IO_FILE_HPP

#include <concepts>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <string>
#include <type_traits>
#include <utility>

namespace thes {
template<typename T>
concept ByteLike =
  std::same_as<T, std::byte> || std::same_as<T, unsigned char> || std::same_as<T, signed char>;
template<typename T>
concept ByteLikePtr = std::is_pointer_v<T> || ByteLike<std::decay_t<T>>;

template<typename T>
concept BufferLike = requires(T& mbuf, const T& cbuf, std::size_t size) {
  { mbuf.resize(size) } -> std::same_as<void>;
  { mbuf.data() } -> ByteLikePtr;
  { mbuf.size() } -> std::convertible_to<std::size_t>;
};

struct FileException : public std::exception {
  explicit FileException(std::string msg) : message_(std::move(msg)) {}

  [[nodiscard]] const char* what() const noexcept override {
    return message_.c_str();
  }

private:
  std::string message_;
};

enum struct Seek : int { set = SEEK_SET, cur = SEEK_CUR, end = SEEK_END };
} // namespace thes

#endif // INCLUDE_THESAUROS_IO_FILE_HPP
