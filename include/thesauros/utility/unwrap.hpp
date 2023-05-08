#ifndef INCLUDE_THESAUROS_UTILITY_UNWRAP_HPP
#define INCLUDE_THESAUROS_UTILITY_UNWRAP_HPP

#include <functional>
#include <utility>

namespace thes {
template<typename T>
inline constexpr T&& unwrap(T&& value) {
  return std::forward<T>(value);
}
template<typename T>
inline constexpr T& unwrap(std::reference_wrapper<T> value) {
  return value;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_UNWRAP_HPP
