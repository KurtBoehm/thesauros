// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_EXECUTION_SYSTEM_SPIN_HPP
#define INCLUDE_THESAUROS_EXECUTION_SYSTEM_SPIN_HPP

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/macropolis/platform.hpp"

namespace thes {
/**
 * Indicates to the CPU that the calling thread is busy-waiting, which allows the processor to
 * optimize accordingly by, for example, lowering the power consumption of the loop.
 * The implementation takes cues from
 * https://github.com/rust-lang/rust/blob/1.98.1/library/core/src/hint.rs#L270 and
 * https://github.com/facebook/folly/blob/v2026.09.21.00/folly/portability/Asm.h.
 */
THES_ALWAYS_INLINE inline void spin_pause() {
  // NOLINTBEGIN(*-no-assembler)
#if THES_X86_64 || THES_X86_32
  // An equivalent of `_mm_pause` that does not require SSE2.
  __builtin_ia32_pause();
#elif THES_ARM64
  // `isb` is the standard choice on AArch64, since `yield` is usually a no-op.
  __asm__ __volatile__("isb" ::: "memory");
#elif THES_ARM32
  // `yield` exists from ARMv6K in the ARM instruction set and from ARMv6T2 and ARMv6-M in Thumb.
#if __ARM_ARCH >= 7 || defined(__ARM_ARCH_6T2__) || defined(__ARM_ARCH_6M__) || \
  ((defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6KZ__)) && !defined(__thumb__))
  __asm__ __volatile__("yield" ::: "memory");
#endif
#elif THES_RISCV
  // `pause` from Zihintpause, spelled out as `fence w, 0` so that assemblers without the extension
  // accept it. Cores without the extension execute it as a `nop`.
  __asm__ __volatile__(".insn i 0x0f, 0, x0, x0, 0x010" ::: "memory");
#elif THES_POWERPC
  // The `yield` hint, which lowers the priority of the hardware thread on SMT cores.
  __asm__ __volatile__("or 27, 27, 27" ::: "memory");
#elif THES_LOONGARCH
  __asm__ __volatile__("ibar 0" ::: "memory");
#elif THES_MIPS
#if __mips_isa_rev >= 2
  __asm__ __volatile__("pause" ::: "memory");
#endif
#endif
  // NOLINTEND(*-no-assembler)
}
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_SYSTEM_SPIN_HPP
