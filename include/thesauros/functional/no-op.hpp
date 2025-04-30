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
template<typename TRet = void>
struct NoOp {
  TRet value{};

  explicit constexpr NoOp(TRet&& val) : value(std::forward<TRet>(val)) {}
  constexpr NoOp()
  requires(std::is_default_constructible_v<TRet>)
  = default;

  constexpr TRet operator()(const auto&... /*args*/) const noexcept {
    return value;
  }
};
template<>
struct NoOp<void> {
  constexpr void operator()(const auto&... /*args*/) const noexcept {}
};
NoOp() -> NoOp<void>;

template<typename TOp>
struct AnyNoOpTrait : public std::false_type {};
template<typename TRet>
struct AnyNoOpTrait<NoOp<TRet>> : public std::true_type {};
template<typename TOp>
concept AnyNoOp = AnyNoOpTrait<TOp>::value;
} // namespace thes

#endif // INCLUDE_THESAUROS_FUNCTIONAL_NO_OP_HPP
