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
#define USER_DEFINED_LITERALS_DEFAULT(TYPE) USER_DEFINED_LITERALS(TYPE, TYPE)

USER_DEFINED_LITERALS(iz, std::make_signed_t<std::size_t>)
USER_DEFINED_LITERALS(uz, std::size_t)
USER_DEFINED_LITERALS(imax, std::intmax_t)
USER_DEFINED_LITERALS(umax, std::uintmax_t)
USER_DEFINED_LITERALS_DEFAULT(i128);
USER_DEFINED_LITERALS_DEFAULT(i16);
USER_DEFINED_LITERALS_DEFAULT(i32);
USER_DEFINED_LITERALS_DEFAULT(i64);
USER_DEFINED_LITERALS_DEFAULT(i8);
USER_DEFINED_LITERALS_DEFAULT(u128);
USER_DEFINED_LITERALS_DEFAULT(u16);
USER_DEFINED_LITERALS_DEFAULT(u32);
USER_DEFINED_LITERALS_DEFAULT(u64);
USER_DEFINED_LITERALS_DEFAULT(u8);

#undef USER_DEFINED_LITERALS_DEFAULT
#undef USER_DEFINED_LITERALS

consteval f32 operator""_f32(long double v) {
  return static_cast<f32>(v);
}
consteval f32 operator""_f32(unsigned long long v) {
  return static_cast<f32>(v);
}
consteval f64 operator""_f64(long double v) {
  return static_cast<f64>(v);
}
consteval f64 operator""_f64(unsigned long long v) {
  return static_cast<f64>(v);
}
} // namespace thes::literals

#endif // INCLUDE_THESAUROS_UTILITY_LITERALS_HPP
