// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_MACROPOLIS_INLINING_HPP
#define INCLUDE_THESAUROS_MACROPOLIS_INLINING_HPP

namespace thes {
#define THES_ALWAYS_INLINE __attribute__((always_inline))
#define THES_NEVER_INLINE __attribute__((noinline))
} // namespace thes

#endif // INCLUDE_THESAUROS_MACROPOLIS_INLINING_HPP
