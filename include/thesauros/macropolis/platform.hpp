// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_MACROPOLIS_PLATFORM_HPP
#define INCLUDE_THESAUROS_MACROPOLIS_PLATFORM_HPP

// These mirror the following references:
//
// https://www.boost.org/doc/libs/latest/libs/predef/doc/index.html
// https://sourceforge.net/p/predef/wiki/Compilers/
// https://sourceforge.net/p/predef/wiki/OperatingSystems/
// https://sourceforge.net/p/predef/wiki/Architectures/
//
// They are designed to be sufficient for GCC and Clang; compatibility with other compilers is, at
// best, a nice-to-have.

#ifdef __clang__
#define THES_CLANG true
#define THES_GCC false
#elifdef __GNUC__
#define THES_CLANG false
#define THES_GCC true
#else
#define THES_CLANG false
#define THES_GCC false
#endif

#ifdef __GNUC__
#define THES_GCC_COMPAT true
#else
#define THES_GCC_COMPAT false
#endif

#ifdef __linux__
#define THES_LINUX true
#else
#define THES_LINUX false
#endif

#ifdef __APPLE__
#define THES_APPLE true
#else
#define THES_APPLE false
#endif

#ifdef _WIN64
#define THES_WINDOWS true
#else
#define THES_WINDOWS false
#endif

#if defined(__x86_64__) || defined(_M_X64)
#define THES_X86_64 true
#else
#define THES_X86_64 false
#endif

#ifdef __i386__
#define THES_X86_32 true
#else
#define THES_X86_32 false
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#define THES_ARM64 true
#else
#define THES_ARM64 false
#endif

#ifdef __arm__
#define THES_ARM32 true
#else
#define THES_ARM32 false
#endif

#ifdef __riscv
#define THES_RISCV true
#else
#define THES_RISCV false
#endif

#ifdef __powerpc__
#define THES_POWERPC true
#else
#define THES_POWERPC false
#endif

// https://github.com/loongson/la-toolchain-conventions/blob/releases/v1.2/LoongArch-toolchain-conventions-EN.adoc#cc-preprocessor-built-in-macro-definitions
#ifdef __loongarch__
#define THES_LOONGARCH true
#else
#define THES_LOONGARCH false
#endif

#ifdef __mips__
#define THES_MIPS true
#else
#define THES_MIPS false
#endif

#endif // INCLUDE_THESAUROS_MACROPOLIS_PLATFORM_HPP
