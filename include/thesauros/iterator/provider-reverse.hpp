// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_ITERATOR_PROVIDER_REVERSE_HPP
#define INCLUDE_THESAUROS_ITERATOR_PROVIDER_REVERSE_HPP

#include <compare>

#include "thesauros/iterator/concepts.hpp"

namespace thes::iter_provider {
template<typename TForwardProvider, typename TDerived>
struct Reverse {
  using IterTypes = TForwardProvider::IterTypes;
  using Ref = IterTypes::IterRef;
  using Diff = IterTypes::IterDiff;

  static constexpr Ref deref(const auto& self)
  requires iter_provider::RevDeref<TDerived, TForwardProvider> ||
           (iter_provider::Deref<TDerived, TForwardProvider> &&
            iter_provider::Decr<TDerived, TForwardProvider>)
  {
    if constexpr (iter_provider::RevDeref<TDerived, TForwardProvider>) {
      return TForwardProvider::rev_deref(self);
    } else {
      auto tmp = self;
      TForwardProvider::decr(tmp);
      return TForwardProvider::deref(tmp);
    }
  }

  static constexpr void incr(auto& self)
  requires iter_provider::Decr<TDerived, TForwardProvider>
  {
    TForwardProvider::decr(self);
  }
  static constexpr void decr(auto& self)
  requires iter_provider::Incr<TDerived, TForwardProvider>
  {
    TForwardProvider::incr(self);
  }
  static constexpr void iadd(auto& self, auto diff)
  requires iter_provider::InPlaceAdd<TDerived, TForwardProvider>
  {
    if constexpr (iter_provider::InPlaceSub<TDerived, TForwardProvider>) {
      TForwardProvider::isub(self, diff);
    } else {
      TForwardProvider::iadd(self, -diff);
    }
  }
  static constexpr void isub(auto& self, auto diff)
  requires iter_provider::InPlaceAdd<TDerived, TForwardProvider>
  {
    TForwardProvider::iadd(self, diff);
  }

  static constexpr bool eq(const auto& i1, const auto& i2)
  requires iter_provider::Eq<TDerived, TForwardProvider>
  {
    return TForwardProvider::eq(i2, i1);
  }
  static constexpr std::strong_ordering three_way(const auto& i1, const auto& i2)
  requires iter_provider::ThreeWayCmp<TDerived, TForwardProvider>
  {
    return TForwardProvider::three_way(i2, i1);
  }

  static constexpr Diff sub(const auto& i1, const auto& i2)
  requires iter_provider::Sub<TDerived, TForwardProvider>
  {
    return TForwardProvider::sub(i2, i1);
  }
};
} // namespace thes::iter_provider

#endif // INCLUDE_THESAUROS_ITERATOR_PROVIDER_REVERSE_HPP
