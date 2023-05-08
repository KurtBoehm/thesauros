#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_REDUCE_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_REDUCE_HPP

#include <cstddef>
#include <type_traits>

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/get-at.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/static-value.hpp"

namespace thes::star {
template<typename TOp, typename TInit>
struct LeftReduceGenerator {
  TOp op;
  TInit init;

  template<typename TRange>
  constexpr auto operator()(TRange&& range) const {
    constexpr std::size_t size = ::thes::star::size<TRange>;
    auto impl = [this, &range](auto& self, auto idx, auto value) {
      if constexpr (idx < size) {
        return self(self, static_auto<idx + 1>, op(value, get_at<idx>(range)));
      } else {
        return value;
      }
    };
    return impl(impl, static_value<std::size_t, 0>, init);
  }
};
template<typename TOp, typename TInit>
struct ConsumerGeneratorTrait<LeftReduceGenerator<TOp, TInit>> : public std::true_type {};

template<typename TOp, typename TInit>
inline constexpr LeftReduceGenerator<TOp, TInit> left_reduce(TOp&& op, TInit&& init) {
  return {std::forward<TOp>(op), std::forward<TInit>(init)};
}

template<typename TOp, typename TInit>
struct RightReduceGenerator {
  TOp op;
  TInit init;

  template<typename TRange>
  constexpr auto operator()(TRange&& range) const {
    constexpr std::size_t size = ::thes::star::size<TRange>;
    auto impl = [this, &range](auto& self, auto idx) {
      if constexpr (idx < size) {
        return op(get_at<idx>(range), self(self, static_auto<idx + 1>));
      } else {
        return init;
      }
    };
    return impl(impl, static_value<std::size_t, 0>);
  }
};
template<typename TOp, typename TInit>
struct ConsumerGeneratorTrait<RightReduceGenerator<TOp, TInit>> : public std::true_type {};

template<typename TOp, typename TInit>
inline constexpr RightReduceGenerator<TOp, TInit> right_reduce(TOp&& op, TInit&& init) {
  return {std::forward<TOp>(op), std::forward<TInit>(init)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_REDUCE_HPP
