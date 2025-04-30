// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_MACROPOLIS_PLATFORM_HPP
#define INCLUDE_THESAUROS_MACROPOLIS_PLATFORM_HPP

namespace thes {
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

#ifdef __SSE__
#define THES_X86_SSE true
#else
#define THES_X86_SSE false
#endif

#ifdef __SSE2__
#define THES_X86_SSE2 true
#else
#define THES_X86_SSE2 false
#endif

#ifdef __SSE4_1__
#define THES_X86_SSE4_1 true
#else
#define THES_X86_SSE4_1 false
#endif

#ifdef __AVX512F__
#define THES_X86_AVX512F true
#else
#define THES_X86_AVX512F false
#endif

#ifdef __AVX512BW__
#define THES_X86_AVX512BW true
#else
#define THES_X86_AVX512BW false
#endif

#ifdef __AVX512VL__
#define THES_X86_AVX512VL true
#else
#define THES_X86_AVX512VL false
#endif

#ifdef __FMA__
#define THES_X86_FMA3 true
#else
#define THES_X86_FMA3 false
#endif
} // namespace thes

#endif // INCLUDE_THESAUROS_MACROPOLIS_PLATFORM_HPP
