#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_REDUCE_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_REDUCE_HPP

#include <algorithm>
#include <cstddef>
#include <type_traits>

#include "thesauros/utility/inlining.hpp"
#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/get-at.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/static-value.hpp"

namespace thes::star {
template<typename TOp, typename TInit, bool tRight>
struct InitReduceGenerator {
  TOp op;
  TInit init;

  template<typename TRange>
  constexpr auto operator()(TRange&& range) const {
    constexpr std::size_t size = thes::star::size<TRange>;
    if constexpr (!tRight) {
      auto impl = [&](auto& self, auto idx, auto value) THES_ALWAYS_INLINE {
        if constexpr (idx < size) {
          return self(self, static_auto<idx + 1>, op(value, get_at<idx>(range)));
        } else {
          return value;
        }
      };
      return impl(impl, static_value<std::size_t, 0>, init);
    } else {
      auto impl = [&](auto& self, auto idx) THES_ALWAYS_INLINE {
        if constexpr (idx < size) {
          return op(get_at<idx>(range), self(self, static_auto<idx + 1>));
        } else {
          return init;
        }
      };
      return impl(impl, static_value<std::size_t, 0>);
    }
  }
};
template<typename TOp, typename TInit, bool tRight>
struct ConsumerGeneratorTrait<InitReduceGenerator<TOp, TInit, tRight>> : public std::true_type {};

template<typename TOp, bool tRight>
struct ReduceGenerator {
  TOp op;

  template<typename TRange>
  constexpr auto operator()(TRange&& range) const {
    constexpr std::size_t size = thes::star::size<TRange>;
    if constexpr (!tRight) {
      auto impl = [&](auto& self, auto idx, auto value) THES_ALWAYS_INLINE {
        if constexpr (idx < size) {
          return self(self, static_auto<idx + 1>, op(value, get_at<idx>(range)));
        } else {
          return value;
        }
      };
      return impl(impl, static_value<std::size_t, 1>, get_at<0>(range));
    } else {
      auto impl = [&](auto& self, auto idx) THES_ALWAYS_INLINE {
        if constexpr (idx + 1 < size) {
          return op(get_at<idx>(range), self(self, static_auto<idx + 1>));
        } else {
          return get_at<idx>(range);
        }
      };
      return impl(impl, static_value<std::size_t, 0>);
    }
  }
};
template<typename TOp, bool tRight>
struct ConsumerGeneratorTrait<ReduceGenerator<TOp, tRight>> : public std::true_type {};

template<typename TOp, typename TInit>
inline constexpr InitReduceGenerator<TOp, TInit, false> left_reduce(TOp&& op, TInit&& init) {
  return {std::forward<TOp>(op), std::forward<TInit>(init)};
}
template<typename TOp>
inline constexpr ReduceGenerator<TOp, false> left_reduce(TOp&& op) {
  return {std::forward<TOp>(op)};
}
inline constexpr auto minimum =
  left_reduce([]<typename T>(const T& v1, const T& v2) { return std::min(v1, v2); });
inline constexpr auto maximum =
  left_reduce([]<typename T>(const T& v1, const T& v2) { return std::max(v1, v2); });

template<typename TOp, typename TInit>
inline constexpr InitReduceGenerator<TOp, TInit, true> right_reduce(TOp&& op, TInit&& init) {
  return {std::forward<TOp>(op), std::forward<TInit>(init)};
}
template<typename TOp>
inline constexpr ReduceGenerator<TOp, true> right_reduce(TOp&& op) {
  return {std::forward<TOp>(op)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_REDUCE_HPP
