#ifndef INCLUDE_THESAUROS_UTILITY_PRIMITIVES_HPP
#define INCLUDE_THESAUROS_UTILITY_PRIMITIVES_HPP

#include <climits>
#include <cstdint>
#include <limits>

namespace thes {
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

using f32 = float;
static_assert(std::numeric_limits<f32>::is_iec559 && sizeof(f32) == 4);
using f64 = double;
static_assert(std::numeric_limits<f64>::is_iec559 && sizeof(f64) == 8);
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_PRIMITIVES_HPP
