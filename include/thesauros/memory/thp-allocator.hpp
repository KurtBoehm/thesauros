#ifndef INCLUDE_THESAUROS_MEMORY_THP_ALLOCATOR_HPP
#define INCLUDE_THESAUROS_MEMORY_THP_ALLOCATOR_HPP

#include <cstddef>
#include <cstdlib>
#include <limits>
#include <new>

#include <sys/mman.h>

namespace thes {
template<typename T>
struct TransparentHugePagesAllocator {
  static constexpr std::size_t huge_page_size = 1U << 21U; // 2 MiB
  using value_type = T;

  TransparentHugePagesAllocator() = default;

  T* allocate(std::size_t n) {
    if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) {
      throw std::bad_alloc{};
    }
    void* p = nullptr;
    posix_memalign(&p, huge_page_size, n * sizeof(T));
    madvise(p, n * sizeof(T), MADV_HUGEPAGE);
    if (p == nullptr) {
      throw std::bad_alloc{};
    }
    return static_cast<T*>(p);
  }

  void deallocate(T* p, std::size_t /*n*/) {
    std::free(p);
  }
};
} // namespace thes

#endif // INCLUDE_THESAUROS_MEMORY_THP_ALLOCATOR_HPP
