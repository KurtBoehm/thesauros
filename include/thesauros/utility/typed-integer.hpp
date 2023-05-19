#ifndef INCLUDE_THESAUROS_UTILITY_TYPED_INTEGER_HPP
#define INCLUDE_THESAUROS_UTILITY_TYPED_INTEGER_HPP

#include <concepts>

namespace thes {
template<std::unsigned_integral T, bool tExplicitCast>
struct TypedInteger {
  explicit constexpr TypedInteger(T value) : value_(value) {}

  [[maybe_unused]] explicit constexpr operator T() const
  requires(tExplicitCast)
  {
    return value_;
  }
  [[maybe_unused]] constexpr operator T() const
  requires(!tExplicitCast)
  {
    return value_;
  }

private:
  T value_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_TYPED_INTEGER_HPP
