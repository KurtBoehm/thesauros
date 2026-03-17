// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_ARRAY_CONSTRUCTION_HPP
#define INCLUDE_THESAUROS_CONTAINERS_ARRAY_CONSTRUCTION_HPP

namespace thes {
struct CopyConstruct {};
struct MoveConstruct {};
struct UninitializedConstruct {};

inline constexpr CopyConstruct copy_construct{};
inline constexpr MoveConstruct move_construct{};
inline constexpr UninitializedConstruct uninitialized_construct{};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_ARRAY_CONSTRUCTION_HPP
