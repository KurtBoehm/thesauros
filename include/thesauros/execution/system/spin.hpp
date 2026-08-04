// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_EXECUTION_SYSTEM_SPIN_HPP
#define INCLUDE_THESAUROS_EXECUTION_SYSTEM_SPIN_HPP

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/macropolis/platform.hpp"

#if THES_X86_64
#include <immintrin.h>
#elif !THES_ARM64
#include <thread>
#endif

namespace thes {
/**
 * Tell the CPU that the calling thread is busy-waiting, which lowers the power consumption of the
 * loop and the cost of leaving it, and falls back to yielding to the OS scheduler on architectures
 * without a suitable instruction.
 */
THES_ALWAYS_INLINE inline void spin_pause() {
#if THES_X86_64
  _mm_pause();
#elif THES_ARM64
  // `isb` is what standard library and other spin-wait implementations use on AArch64, since
  // `yield` is a no-op on most cores.
  __asm__ __volatile__("isb" ::: "memory"); // NOLINT
#else
  std::this_thread::yield();
#endif
}
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_SYSTEM_SPIN_HPP
