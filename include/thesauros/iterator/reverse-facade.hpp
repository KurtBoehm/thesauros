// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_ITERATOR_REVERSE_FACADE_HPP
#define INCLUDE_THESAUROS_ITERATOR_REVERSE_FACADE_HPP

#include <compare>

#include "thesauros/iterator/facade.hpp"

namespace thes {
/**
 * A CRTP mixin that reverses a `Base` iterator. `Derived` exposes the wrapped `Base` instance via
 * `base()`/`base() const`, and `ReverseFacade` derives the reversed primitives from `Base`’s own
 * primitives by swapping `incr`/`decr`, negating `iadd`/`isub` and flipping the comparisons.
 *
 * `Base` may provide `rev_deref()` as a cheaper alternative to decrementing a copy before
 * dereferencing, mirroring `std::reverse_iterator`’s behavior.
 */
template<typename IterTypes>
struct ReverseIteratorFacade : IteratorFacade<IterTypes> {
  using Diff = IterTypes::IterDiff;

  constexpr decltype(auto) deref(this const auto& self)
  requires(
    requires { self.rev_deref(); } ||
    requires(std::decay_t<decltype(self)> tmp) {
      *self.base();
      --tmp.base();
    })
  {
    if constexpr (requires { self.rev_deref(); }) {
      return self.rev_deref();
    } else {
      auto tmp = self.base();
      --tmp;
      return *tmp;
    }
  }

  constexpr void incr(this auto& self)
  requires(requires { --self.base(); })
  {
    --self.base();
  }
  constexpr void decr(this auto& self)
  requires(requires { ++self.base(); })
  {
    ++self.base();
  }

  constexpr void iadd(this auto& self, Diff diff)
  requires(
    requires { self.base() -= diff; } || requires { self.base() += -diff; })
  {
    if constexpr (requires { self.base() -= diff; }) {
      self.base() -= diff;
    } else {
      self.base() += -diff;
    }
  }
  constexpr void isub(this auto& self, Diff diff)
  requires(requires { self.base() += diff; })
  {
    self.base() += diff;
  }

  constexpr bool eq(this const auto& self, const auto& other)
  requires(requires { self.base() == other.base(); })
  {
    return self.base() == other.base();
  }
  template<typename Derived>
  constexpr std::strong_ordering three_way(this const Derived& self, const Derived& other)
  requires(requires { other.base() <=> self.base(); })
  {
    return other.base() <=> self.base();
  }
  template<typename Derived>
  constexpr Diff sub(this const Derived& self, const Derived& other)
  requires(requires { other.base() - self.base(); })
  {
    return other.base() - self.base();
  }
};
} // namespace thes

#endif // INCLUDE_THESAUROS_ITERATOR_REVERSE_FACADE_HPP
