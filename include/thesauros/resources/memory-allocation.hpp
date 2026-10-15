// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_RESOURCES_MEMORY_ALLOCATION_HPP
#define INCLUDE_THESAUROS_RESOURCES_MEMORY_ALLOCATION_HPP

#include "thesauros/macropolis/platform.hpp"

#if THES_GLIBC
#include <malloc.h>
#endif

namespace thes {
/**
 * Configure memory allocation appropriately for highly parallel applications with large memory
 * allocations to reduce the memory overhead associated with dynamic memory allocation.
 *
 * In practice, this function is a no-op apart from compilation with glibc, where the allocation
 * options are adjusted to limit both the size of allocations placed into arenas
 * (`M_MMAP_THRESHOLD`) and the size of the free space in an arena before it is handed back to the
 * operating system (`M_TRIM_THRESHOLD`). If both of these are a few dozen megabytes in size, which
 * they are dynamically increased to by default “to suit the allocation patterns of the program”,
 * the overhead can grow to a few gigabytes with 128 or 256 threads. This function fixes both at
 * their default values of 128 KiB, which disables dynamic adjustments and alleviates this issue
 * without impacting runtime performance in my testing.
 *
 * Details can be found at
 * https://sourceware.org/glibc/manual/2.44/html_node/Malloc-Tunable-Parameters.html.
 */
inline void configure_malloc() noexcept {
#if THES_GLIBC && defined(M_MMAP_THRESHOLD) && defined(M_TRIM_THRESHOLD)
  mallopt(M_MMAP_THRESHOLD, 131072);
  mallopt(M_TRIM_THRESHOLD, 131072);
#endif
}
} // namespace thes

#endif // INCLUDE_THESAUROS_RESOURCES_MEMORY_ALLOCATION_HPP
