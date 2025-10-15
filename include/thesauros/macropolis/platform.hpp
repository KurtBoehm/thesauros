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
} // namespace thes

#endif // INCLUDE_THESAUROS_MACROPOLIS_PLATFORM_HPP
