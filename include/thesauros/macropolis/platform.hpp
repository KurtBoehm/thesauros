// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_MACROPOLIS_PLATFORM_HPP
#define INCLUDE_THESAUROS_MACROPOLIS_PLATFORM_HPP

#ifdef __clang__
#define THES_CLANG true
#else
#define THES_CLANG false
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

#if defined(__aarch64__) || defined(_M_ARM64)
#define THES_ARM64 true
#else
#define THES_ARM64 false
#endif

#endif // INCLUDE_THESAUROS_MACROPOLIS_PLATFORM_HPP
