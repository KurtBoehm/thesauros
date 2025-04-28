#ifndef INCLUDE_THESAUROS_TYPES_LITERALS_HPP
#define INCLUDE_THESAUROS_TYPES_LITERALS_HPP

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "thesauros/charconv/parse-integer.hpp"
#include "thesauros/types/primitives.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes::literals {
#define USER_DEFINED_LITERALS(OP_NAME, TYPE) \
  consteval TYPE operator""_##OP_NAME(const char* ptr) { \
    return parse_integer<TYPE>(ptr, thes::auto_tag<IntegerParseMode::literal>).value(); \
  } \
  consteval TYPE operator""_##OP_NAME(const char* ptr, std::size_t len) { \
    return parse_integer<TYPE>({ptr, len}, thes::auto_tag<IntegerParseMode::extended>).value(); \
  }

USER_DEFINED_LITERALS(iz, std::make_signed_t<std::size_t>)
USER_DEFINED_LITERALS(uz, std::size_t)
USER_DEFINED_LITERALS(imax, std::intmax_t)
USER_DEFINED_LITERALS(umax, std::uintmax_t)
USER_DEFINED_LITERALS(i128, i128);
USER_DEFINED_LITERALS(i16, i16);
USER_DEFINED_LITERALS(i32, i32);
USER_DEFINED_LITERALS(i64, i64);
USER_DEFINED_LITERALS(i8, i8);
USER_DEFINED_LITERALS(u128, u128);
USER_DEFINED_LITERALS(u16, u16);
USER_DEFINED_LITERALS(u32, u32);
USER_DEFINED_LITERALS(u64, u64);
USER_DEFINED_LITERALS(u8, u8);

#undef USER_DEFINED_LITERALS

#define USER_DEFINED_LITERALS(WIDTH) \
  consteval f##WIDTH operator""_f##WIDTH(long double v) { \
    return static_cast<f##WIDTH>(v); \
  } \
  consteval f##WIDTH operator""_f##WIDTH(unsigned long long v) { \
    return static_cast<f##WIDTH>(v); \
  }

USER_DEFINED_LITERALS(16)
USER_DEFINED_LITERALS(32)
USER_DEFINED_LITERALS(64)

#undef USER_DEFINED_LITERALS
} // namespace thes::literals

#endif // INCLUDE_THESAUROS_TYPES_LITERALS_HPP
