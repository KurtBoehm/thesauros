#ifndef INCLUDE_THESAUROS_STATIC_RANGES_RANGES_GENERATE_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_RANGES_GENERATE_HPP

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"

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

template<std::size_t tSize, typename TRet, std::invocable<> TGen>
struct Generate : public generate_impl::ValueBase<TRet, TGen> {
  explicit constexpr Generate(TGen&& gen) : gen_{std::forward<TGen>(gen)} {}

  static constexpr std::size_t size = tSize;

  template<std::size_t tIndex>
  requires(tIndex < tSize)
  THES_ALWAYS_INLINE constexpr auto get() const {
    return gen_();
  }

private:
  TGen gen_;
};

template<std::size_t tSize, std::invocable<> TGen>
THES_ALWAYS_INLINE inline constexpr auto generate(TGen&& gen) {
  return Generate<tSize, void, TGen>{std::forward<TGen>(gen)};
}
template<typename TRet, std::size_t tSize, std::invocable<> TGen>
THES_ALWAYS_INLINE inline constexpr auto generate(TGen&& gen) {
  return Generate<tSize, TRet, TGen>{std::forward<TGen>(gen)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_RANGES_GENERATE_HPP
