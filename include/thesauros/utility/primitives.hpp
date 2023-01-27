#ifndef INCLUDE_THESAUROS_UTILITY_PRIMITIVES_HPP
#define INCLUDE_THESAUROS_UTILITY_PRIMITIVES_HPP

#include <climits>
#include <cstdint>
#include <limits>

namespace thes {
using U8 = std::uint8_t;
using U16 = std::uint16_t;
using U32 = std::uint32_t;
using U64 = std::uint64_t;
using I8 = std::int8_t;
using I16 = std::int16_t;
using I32 = std::int32_t;
using I64 = std::int64_t;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
using I128 = __int128;
using U128 = unsigned __int128;
#pragma GCC diagnostic pop

using F32 = float;
static_assert(std::numeric_limits<F32>::is_iec559 and sizeof(F32) == 4);
using F64 = double;
static_assert(std::numeric_limits<F64>::is_iec559 and sizeof(F64) == 8);
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_PRIMITIVES_HPP
