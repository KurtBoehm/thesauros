// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_FUNCTIONAL_NO_OP_HPP
#define INCLUDE_THESAUROS_FUNCTIONAL_NO_OP_HPP

#include <type_traits>
#include <utility>

namespace thes {
template<typename Ret = void>
struct NoOp {
  Ret value{};

  explicit constexpr NoOp(Ret&& val) : value{std::forward<Ret>(val)} {}
  constexpr NoOp()
  requires(std::is_default_constructible_v<Ret>)
  = default;

  constexpr Ret operator()(const auto&... /*args*/) const noexcept {
    return value;
  }
};
template<>
struct NoOp<void> {
  constexpr void operator()(const auto&... /*args*/) const noexcept {}
};
NoOp() -> NoOp<void>;

template<typename F>
inline constexpr bool is_no_op = false;
template<typename Ret>
inline constexpr bool is_no_op<NoOp<Ret>> = true;
template<typename F>
concept AnyNoOp = is_no_op<F>;
} // namespace thes

#endif // INCLUDE_THESAUROS_FUNCTIONAL_NO_OP_HPP
