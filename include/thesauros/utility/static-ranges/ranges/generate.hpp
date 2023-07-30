#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_GENERATE_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_GENERATE_HPP

#include <concepts>
#include <cstddef>
#include <utility>

namespace thes::star {
template<std::size_t tSize, std::invocable<> TFun>
struct Generate {
  TFun fun;

  static constexpr std::size_t size = tSize;

  template<std::size_t tIndex>
  requires(tIndex < tSize)
  constexpr auto get() const {
    return fun();
  }
};

template<std::size_t tSize, std::invocable<> TFun>
inline constexpr auto generate(TFun&& fun) {
  return Generate<tSize, TFun>{std::forward<TFun>(fun)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_GENERATE_HPP
