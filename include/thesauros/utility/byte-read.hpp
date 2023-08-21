#ifndef INCLUDE_THESAUROS_UTILITY_BYTE_READ_HPP
#define INCLUDE_THESAUROS_UTILITY_BYTE_READ_HPP

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

#endif // INCLUDE_THESAUROS_UTILITY_BYTE_READ_HPP
