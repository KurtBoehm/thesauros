#ifndef INCLUDE_THESAUROS_MATH_ARITHMETIC_HPP
#define INCLUDE_THESAUROS_MATH_ARITHMETIC_HPP

#include <limits>

namespace thes {
template<typename T>
inline constexpr T div_ceil(const T dividend, const T divisor) {
  return dividend / divisor + T{dividend % divisor != 0};
}

template<typename T>
inline constexpr T prod_div(const T factor1, const T factor2, const T divisor) {
  assert(divisor > 0);
  assert(factor2 <= std::numeric_limits<T>::max() / divisor);

  const T div = factor1 / divisor;
  const T rem = factor1 % divisor;
  return factor2 * div + (factor2 * rem) / divisor;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_MATH_ARITHMETIC_HPP
