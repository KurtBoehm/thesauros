// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONCEPTS_NUMERIC_HPP
#define INCLUDE_THESAUROS_CONCEPTS_NUMERIC_HPP

#include <bit>

#include "thesauros/concepts/fundamental.hpp"

namespace thes {
template<auto N>
concept PowerOfTwo = UnsignedInteger<decltype(N)> && std::has_single_bit(N);
}

#endif // INCLUDE_THESAUROS_CONCEPTS_NUMERIC_HPP
