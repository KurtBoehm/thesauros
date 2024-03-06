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

template<std::size_t tSize, typename TRet, std::invocable<> TGen>
struct Generate : public generate_impl::ValueBase<TRet, TGen> {
  explicit Generate(TGen&& gen) : gen_{std::forward<TGen>(gen)} {}

  static constexpr std::size_t size = tSize;

  template<std::size_t tIndex>
  requires(tIndex < tSize)
  constexpr auto get() const {
    return gen_();
  }

private:
  TGen gen_;
};

template<std::size_t tSize, std::invocable<> TGen>
inline constexpr auto generate(TGen&& gen) {
  return Generate<tSize, void, TGen>{std::forward<TGen>(gen)};
}
template<typename TRet, std::size_t tSize, std::invocable<> TGen>
inline constexpr auto generate(TGen&& gen) {
  return Generate<tSize, TRet, TGen>{std::forward<TGen>(gen)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_GENERATE_HPP
