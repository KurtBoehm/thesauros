#ifndef INCLUDE_THESAUROS_RESOURCES_PREFETCH_HPP
#define INCLUDE_THESAUROS_RESOURCES_PREFETCH_HPP

#include <cstddef>

#include "thesauros/macropolis/inlining.hpp"

namespace thes {
// 64 bytes is most common on modern systems, but this might be updated if the need arises.
inline constexpr std::size_t cache_line_size = 64;

template<std::size_t tCacheLineNum = 1>
requires(tCacheLineNum >= 1)
THES_ALWAYS_INLINE inline void prefetch(const void* addr) {
  __builtin_prefetch(addr);
  if constexpr (tCacheLineNum > 1) {
    prefetch<tCacheLineNum - 1>(static_cast<const char*>(addr) + cache_line_size);
  }
}
} // namespace thes

#endif // INCLUDE_THESAUROS_RESOURCES_PREFETCH_HPP
