// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_TYPES_GENERATE_TAG_HPP
#define INCLUDE_THESAUROS_TYPES_GENERATE_TAG_HPP

#include <cstddef>

namespace thes {
/** A tag designed to distinguish constructors that construct their components from a generator. */
struct GenerateTag {};
inline constexpr GenerateTag generate_tag{};
} // namespace thes

#endif // INCLUDE_THESAUROS_TYPES_GENERATE_TAG_HPP
