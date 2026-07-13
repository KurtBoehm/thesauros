// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_ITERATOR_STATE_FACADE_HPP
#define INCLUDE_THESAUROS_ITERATOR_STATE_FACADE_HPP

#include <compare>
#include <concepts>

#include "thesauros/iterator/facade.hpp"
#include "thesauros/math/integer-cast.hpp"
#include "thesauros/utility/integral-value.hpp"

namespace thes {
/**
 * A CRTP mixin that implements an iterator entirely in terms of a single, comparable “state”
 * value exposed by the inheriting type via `state()`/`state() const`. The inheriting type must
 * additionally provide `value()`.
 *
 * The iterator category is derived automatically from the operations `State` supports: incrementing
 * enables the forward operators; decrementing enables the bidirectional operators; in-place
 * addition together with subtraction and three-way comparison enable the random-access operators.
 *
 * The inheriting type may optionally provide `test_if_cmp(const Self&) const`, called before
 * every comparison, e.g. to assert that both iterators refer to the same range.
 */
template<typename IterTypes>
struct StateIteratorFacade : IteratorFacade<IterTypes> {
  using Base = IteratorFacade<IterTypes>;

  using Diff = IterTypes::IterDiff;

  //------------------------------------------------------------------------------------------------
  // Forward and bidirectional primitives
  //------------------------------------------------------------------------------------------------

  constexpr decltype(auto) deref(this auto& self) {
    return self.value();
  }

  constexpr void incr(this auto& self)
  requires(requires { ++self.state(); })
  {
    ++self.state();
  }
  constexpr void decr(this auto& self)
  requires(requires { --self.state(); })
  {
    --self.state();
  }
  template<typename Derived>
  constexpr bool eq(this const Derived& self, const Derived& other)
  requires(requires { self.state() == other.state(); })
  {
    self.check_cmp(other);
    return self.state() == other.state();
  }

  //------------------------------------------------------------------------------------------------
  // Random-access primitives
  //------------------------------------------------------------------------------------------------

  constexpr void iadd(this auto& self, Diff diff)
  requires(requires { self.state() += diff; })
  {
    using State = std::decay_t<decltype(self.state())>;
    if constexpr (std::integral<State>) {
      if (diff < Diff{0}) {
        self.state() -= *safe_cast<State>(integral_value(-diff));
      } else {
        self.state() += *safe_cast<State>(integral_value(diff));
      }
    } else {
      self.state() += diff;
    }
  }
  constexpr void isub(this auto& self, Diff diff)
  requires(requires { self.state() -= diff; })
  {
    using State = std::decay_t<decltype(self.state())>;
    if constexpr (std::integral<State>) {
      if (diff < Diff{0}) {
        self.state() += *safe_cast<State>(integral_value(-diff));
      } else {
        self.state() -= *safe_cast<State>(integral_value(diff));
      }
    } else {
      self.state() -= diff;
    }
  }

  template<typename Derived>
  constexpr Diff sub(this const Derived& self, const Derived& other)
  requires(requires { self.state() - other.state(); })
  {
    using State = std::decay_t<decltype(self.state())>;
    self.check_cmp(other);
    if constexpr (std::integral<State>) {
      return *safe_cast<Diff>(self.state()) - *safe_cast<Diff>(other.state());
    } else {
      return self.state() - other.state();
    }
  }
  template<typename Derived>
  constexpr std::strong_ordering three_way(this const Derived& self, const Derived& other)
  requires(requires { self.state() <=> other.state(); })
  {
    self.check_cmp(other);
    return self.state() <=> other.state();
  }

private:
  /** Calls `Derived::test_if_cmp`, if provided. */
  template<typename Derived>
  constexpr void check_cmp(this const Derived& self, const Derived& other) {
    if constexpr (requires { self.test_if_cmp(other); }) {
      self.test_if_cmp(other);
    }
  }
};
} // namespace thes

#endif // INCLUDE_THESAUROS_ITERATOR_STATE_FACADE_HPP
