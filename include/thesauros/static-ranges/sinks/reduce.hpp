// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_STATIC_RANGES_SINKS_REDUCE_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_SINKS_REDUCE_HPP

#include <algorithm>
#include <cstddef>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/get-at.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes::star {
template<typename BinOp, typename Init, bool Right>
struct InitReduceGenerator : public ConsumerGeneratorBase {
  BinOp binary_op;
  Init initial;

  constexpr InitReduceGenerator(BinOp&& op, Init&& init)
      : binary_op(std::forward<BinOp>(op)), initial(std::forward<Init>(init)) {}

  template<typename Range>
  THES_ALWAYS_INLINE constexpr auto operator()(Range&& range) const {
    constexpr std::size_t size = thes::star::size<Range>;
    if constexpr (!Right) {
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

template<typename BinOp, bool Right>
struct ReduceGenerator : public ConsumerGeneratorBase {
  BinOp binary_op;

  explicit constexpr ReduceGenerator(BinOp&& op) : binary_op(std::forward<BinOp>(op)) {}

  template<typename Range>
  THES_ALWAYS_INLINE constexpr auto operator()(Range&& range) const {
    constexpr std::size_t size = thes::star::size<Range>;
    if constexpr (!Right) {
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

template<typename BinOp, typename Init>
constexpr InitReduceGenerator<BinOp, Init, false> left_reduce(BinOp&& op, Init&& init) {
  return {std::forward<BinOp>(op), std::forward<Init>(init)};
}
template<typename BinOp>
constexpr auto left_reduce(BinOp&& op) {
  return ReduceGenerator<BinOp, false>{std::forward<BinOp>(op)};
}
inline constexpr auto minimum =
  left_reduce([]<typename T>(const T& v1, const T& v2) { return std::min(v1, v2); });
inline constexpr auto maximum =
  left_reduce([]<typename T>(const T& v1, const T& v2) { return std::max(v1, v2); });

template<typename BinOp, typename Init>
constexpr InitReduceGenerator<BinOp, Init, true> right_reduce(BinOp&& op, Init&& init) {
  return {std::forward<BinOp>(op), std::forward<Init>(init)};
}
template<typename BinOp>
constexpr auto right_reduce(BinOp&& op) {
  return ReduceGenerator<BinOp, true>{std::forward<BinOp>(op)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_SINKS_REDUCE_HPP
