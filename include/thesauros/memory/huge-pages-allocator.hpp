// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_MEMORY_HUGE_PAGES_ALLOCATOR_HPP
#define INCLUDE_THESAUROS_MEMORY_HUGE_PAGES_ALLOCATOR_HPP

#include <cstddef>
#include <cstdlib>
#include <limits>
#include <new>

#include "thesauros/macropolis/platform.hpp"

#if THES_LINUX || THES_APPLE
#include <sys/mman.h>
#endif

namespace thes {
template<typename T>
struct HugePagesAllocator {
  static constexpr std::size_t huge_page_size = 1U << 21U; // 2 MiB
  using value_type = T;

  HugePagesAllocator() = default;

  T* allocate(std::size_t n) {
    if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) {
      throw std::bad_alloc{};
    }
    void* p = nullptr;
#if THES_LINUX || THES_APPLE
    posix_memalign(&p, huge_page_size, n * sizeof(T));
#elif THES_WINDOWS
    p = _aligned_malloc(n * sizeof(T), huge_page_size);
#endif
#if THES_LINUX
    madvise(p, n * sizeof(T), MADV_HUGEPAGE);
#endif
    if (p == nullptr) {
      throw std::bad_alloc{};
    }
    return static_cast<T*>(p);
  }

  void deallocate(T* p, std::size_t /*n*/) {
#if THES_LINUX || THES_APPLE
    std::free(p);
#elif THES_WINDOWS
    _aligned_free(p);
#endif
  }
};
} // namespace thes

#endif // INCLUDE_THESAUROS_MEMORY_HUGE_PAGES_ALLOCATOR_HPP
