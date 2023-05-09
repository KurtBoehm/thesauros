#ifndef INCLUDE_THESAUROS_NUMERICS_ARITHMETIC_HPP
#define INCLUDE_THESAUROS_NUMERICS_ARITHMETIC_HPP

namespace thes {
template<typename T>
inline constexpr T div_ceil(const T dividend, const T divisor) {
  return dividend / divisor + T{dividend % divisor != 0};
}
} // namespace thes

#endif // INCLUDE_THESAUROS_NUMERICS_ARITHMETIC_HPP
