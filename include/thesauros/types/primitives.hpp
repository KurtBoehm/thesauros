// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_TYPES_PRIMITIVES_HPP
#define INCLUDE_THESAUROS_TYPES_PRIMITIVES_HPP

#include <cstdint>
#include <limits>

namespace thes {
namespace primitives {
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
using i128 = __int128;
using u128 = unsigned __int128;
#pragma GCC diagnostic pop

using f16 = _Float16;
static_assert(sizeof(f16) == 2);
using f32 = float;
static_assert(std::numeric_limits<f32>::is_iec559 && sizeof(f32) == 4);
using f64 = double;
static_assert(std::numeric_limits<f64>::is_iec559 && sizeof(f64) == 8);
} // namespace primitives

using namespace primitives;
} // namespace thes

#endif // INCLUDE_THESAUROS_TYPES_PRIMITIVES_HPP
