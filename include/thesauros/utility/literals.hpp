#ifndef INCLUDE_THESAUROS_UTILITY_LITERALS_HPP
#define INCLUDE_THESAUROS_UTILITY_LITERALS_HPP

#include <cstdint>
#include <type_traits>

#include "parse-integer.hpp"
#include "primitives.hpp"

namespace thes::literals {
#define USER_DEFINED_LITERALS(OP_NAME, TYPE) \
  consteval TYPE operator""_##OP_NAME(const char* ptr) { \
    return parse_integer<TYPE, false>(ptr).value(); \
  } \
  consteval TYPE operator""_##OP_NAME(const char* ptr, std::size_t len) { \
    return parse_integer<TYPE, true>({ptr, len}).value(); \
  }

USER_DEFINED_LITERALS(iz, std::make_signed_t<std::size_t>)
USER_DEFINED_LITERALS(uz, std::size_t)
USER_DEFINED_LITERALS(imax, std::intmax_t)
USER_DEFINED_LITERALS(umax, std::uintmax_t)
USER_DEFINED_LITERALS(i128, I128);
USER_DEFINED_LITERALS(i16, I16);
USER_DEFINED_LITERALS(i32, I32);
USER_DEFINED_LITERALS(i64, I64);
USER_DEFINED_LITERALS(i8, I8);
USER_DEFINED_LITERALS(u128, U128);
USER_DEFINED_LITERALS(u16, U16);
USER_DEFINED_LITERALS(u32, U32);
USER_DEFINED_LITERALS(u64, U64);
USER_DEFINED_LITERALS(u8, U8);

#undef USER_DEFINED_LITERALS

consteval F32 operator""_f32(long double v) {
  return static_cast<F32>(v);
}
consteval F32 operator""_f32(unsigned long long v) {
  return static_cast<F32>(v);
}
consteval F64 operator""_f64(long double v) {
  return static_cast<F64>(v);
}
consteval F64 operator""_f64(unsigned long long v) {
  return static_cast<F64>(v);
}
} // namespace thes::literals

#endif // INCLUDE_THESAUROS_UTILITY_LITERALS_HPP
