#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_CONSTANT_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_CONSTANT_HPP

#include <cstddef>
#include <utility>

namespace thes::star {
template<std::size_t tSize, typename T>
struct Constant {
  T value;

  static constexpr std::size_t size = tSize;

  template<std::size_t tIndex>
  requires(tIndex < tSize)
  constexpr auto get() const {
    return value;
  }
};

template<std::size_t tSize, typename T>
inline constexpr auto constant(T&& value) {
  return Constant<tSize, T>{std::forward<T>(value)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_CONSTANT_HPP
