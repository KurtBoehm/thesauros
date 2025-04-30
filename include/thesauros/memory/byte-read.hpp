// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_MEMORY_BYTE_READ_HPP
#define INCLUDE_THESAUROS_MEMORY_BYTE_READ_HPP

#include <cstddef>
#include <cstring>
#include <type_traits>

namespace thes {
template<typename T>
requires std::is_trivial_v<T>
inline T byte_read(std::byte* ptr) {
  T v{};
  std::memcpy(&v, ptr, sizeof(T));
  return v;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_MEMORY_BYTE_READ_HPP
