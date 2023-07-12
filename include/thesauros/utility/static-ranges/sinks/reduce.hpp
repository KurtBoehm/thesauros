#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_REDUCE_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_REDUCE_HPP

#include <algorithm>
#include <cstddef>
#include <type_traits>

#include "thesauros/utility/inlining.hpp"
#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/get-at.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/value-tag.hpp"

namespace thes::star {
template<typename TBinOp, typename TInit, bool tRight>
struct InitReduceGenerator : public ConsumerGeneratorBase {
  TBinOp binary_op;
  TInit initial;

  constexpr InitReduceGenerator(TBinOp&& op, TInit&& init)
      : binary_op(std::forward<TBinOp>(op)), initial(std::forward<TInit>(init)) {}

  template<typename TRange>
  constexpr auto operator()(TRange&& range) const {
    constexpr std::size_t size = thes::star::size<TRange>;
    if constexpr (!tRight) {
      auto impl = [&](auto& self, auto idx, auto value) THES_ALWAYS_INLINE {
        if constexpr (idx < size) {
          return self(self, index_tag<idx + 1>, binary_op(value, get_at<idx>(range)));
        } else {
          return value;
        }
      };
      return impl(impl, index_tag<0>, initial);
    } else {
      auto impl = [&](auto& self, auto idx) THES_ALWAYS_INLINE {
        if constexpr (idx < size) {
          return binary_op(get_at<idx>(range), self(self, index_tag<idx + 1>));
        } else {
          return initial;
        }
      };
      return impl(impl, index_tag<0>);
    }
  }
};

template<typename TBinOp, bool tRight>
struct ReduceGenerator : public ConsumerGeneratorBase {
  TBinOp binary_op;

  explicit constexpr ReduceGenerator(TBinOp&& op) : binary_op(std::forward<TBinOp>(op)) {}

  template<typename TRange>
  constexpr auto operator()(TRange&& range) const {
    constexpr std::size_t size = thes::star::size<TRange>;
    if constexpr (!tRight) {
      auto impl = [&](auto& self, auto idx, auto value) THES_ALWAYS_INLINE {
        if constexpr (idx < size) {
          return self(self, index_tag<idx + 1>, binary_op(value, get_at<idx>(range)));
        } else {
          return value;
        }
      };
      return impl(impl, index_tag<1>, get_at<0>(range));
    } else {
      auto impl = [&](auto& self, auto idx) THES_ALWAYS_INLINE {
        if constexpr (idx + 1 < size) {
          return binary_op(get_at<idx>(range), self(self, index_tag<idx + 1>));
        } else {
          return get_at<idx>(range);
        }
      };
      return impl(impl, index_tag<0>);
    }
  }
};

template<typename TBinOp, typename TInit>
inline constexpr InitReduceGenerator<TBinOp, TInit, false> left_reduce(TBinOp&& op, TInit&& init) {
  return {std::forward<TBinOp>(op), std::forward<TInit>(init)};
}
template<typename TBinOp>
inline constexpr auto left_reduce(TBinOp&& op) {
  return ReduceGenerator<TBinOp, false>{std::forward<TBinOp>(op)};
}
inline constexpr auto minimum =
  left_reduce([]<typename T>(const T& v1, const T& v2) { return std::min(v1, v2); });
inline constexpr auto maximum =
  left_reduce([]<typename T>(const T& v1, const T& v2) { return std::max(v1, v2); });

template<typename TBinOp, typename TInit>
inline constexpr InitReduceGenerator<TBinOp, TInit, true> right_reduce(TBinOp&& op, TInit&& init) {
  return {std::forward<TBinOp>(op), std::forward<TInit>(init)};
}
template<typename TBinOp>
inline constexpr auto right_reduce(TBinOp&& op) {
  return ReduceGenerator<TBinOp, true>{std::forward<TBinOp>(op)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_SINKS_REDUCE_HPP
