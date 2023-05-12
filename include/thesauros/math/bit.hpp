#ifndef INCLUDE_THESAUROS_MATH_BIT_HPP
#define INCLUDE_THESAUROS_MATH_BIT_HPP

#include <bit>
#include <concepts>

namespace thes {
template<typename T>
requires std::unsigned_integral<T>
inline constexpr unsigned bit_width(T x) noexcept {
  return static_cast<unsigned>(std::bit_width(x));
}
} // namespace thes

#endif // INCLUDE_THESAUROS_MATH_BIT_HPP
