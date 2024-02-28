#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_GENERATE_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_GENERATE_HPP

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace thes::star {
namespace generate_impl {
template<typename TRet, typename TFun>
struct ValueBase {};

template<typename TRet, typename TFun>
requires(!std::is_void_v<TRet>)
struct ValueBase<TRet, TFun> {
  using Value = TRet;
};
} // namespace generate_impl

template<std::size_t tSize, typename TRet, std::invocable<> TFun>
struct Generate : public generate_impl::ValueBase<TRet, TFun> {
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
  return Generate<tSize, void, TFun>{.fun = std::forward<TFun>(fun)};
}
template<typename TRet, std::size_t tSize, std::invocable<> TFun>
inline constexpr auto generate(TFun&& fun) {
  return Generate<tSize, TRet, TFun>{.fun = std::forward<TFun>(fun)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_GENERATE_HPP
