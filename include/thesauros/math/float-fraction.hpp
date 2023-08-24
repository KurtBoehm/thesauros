#ifndef INCLUDE_THESAUROS_MATH_FLOAT_FRACTION_HPP
#define INCLUDE_THESAUROS_MATH_FLOAT_FRACTION_HPP

#include <concepts>

namespace thes {
template<std::floating_point TFloat>
struct FloatFraction {
  using Float = TFloat;

  constexpr FloatFraction(Float num, Float denom) : numerator(num), denominator(denom) {}

  friend constexpr FloatFraction operator*(FloatFraction f1, Float f2) {
    return {f1.numerator * f2, f1.denominator};
  }
  friend constexpr FloatFraction operator*(Float f1, FloatFraction f2) {
    return f2 * f1;
  }

  constexpr Float to_float() const {
    return numerator / denominator;
  }

  Float numerator;
  Float denominator;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_MATH_FLOAT_FRACTION_HPP
