// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_ITERATOR_FACADE_HPP
#define INCLUDE_THESAUROS_ITERATOR_FACADE_HPP

#include <compare>
#include <concepts>

#include "thesauros/utility/arrow-proxy.hpp"

namespace thes {
//==================================================================================================
// Iterator value type traits
//==================================================================================================

namespace iter {
/** The default `IterTypes` for an iterator dereferencing to a reference. */
template<typename Value, typename Diff>
struct DefaultTypes {
  using IterValue = Value;
  using IterRef = Value&;
  using IterPtr = Value*;
  using IterDiff = Diff;
};

/**
 * The `IterTypes` for an iterator dereferencing to a value, using `ArrowProxy` for `operator->`.
 */
template<typename Value, typename Diff>
struct ValueTypes {
  using IterValue = Value;
  using IterRef = Value;
  using IterPtr = ArrowProxy<Value>;
  using IterDiff = Diff;
};

/**
 * The `IterTypes` for an iterator with split value and reference types, using `ArrowProxy<Value>`
 * for `operator->`.
 */
template<typename Value, typename Ref, typename Diff>
struct ValueRefTypes {
  using IterValue = Value;
  using IterRef = Ref;
  using IterPtr = ArrowProxy<Value>;
  using IterDiff = Diff;
};

/** The `IterTypes` for an iterator that does not support dereferencing. */
template<typename Diff>
struct VoidTypes {
  using IterValue = void;
  using IterRef = void;
  using IterPtr = void;
  using IterDiff = Diff;
};
} // namespace iter

//==================================================================================================
// Iterator facade
//==================================================================================================

/**
 * A CRTP base that implements iterator operators in terms of primitive operations supplied by the
 * inheriting type using deduced `this`.
 *
 * The inheriting type must be at least a forward iterator, i.e. it must provide:
 * - `deref() const -> convertible_to<reference>`
 * - `incr()`
 * - `eq(const Self&) const -> convertible_to<bool>`
 *
 * For a bidirectional iterator, it must additionally provide `decr()`.
 *
 * For a random-access iterator, it must additionally provide:
 * - `iadd(difference_type)`
 * - `sub(const Self&) const -> convertible_to<difference_type>`
 * - `three_way(const Self&) const -> convertible_to<std::strong_ordering>`
 *
 * The following may optionally be provided to replace default implementations:
 * - `isub(difference_type)`, used for `operator-=`/`operator-`; defaults to `iadd(-n)`.
 * - `get_item(difference_type) const`, used for `operator[]`; defaults to `iadd` and `deref`.
 *
 * If the primitive operations above are not public, the inheriting type should declare
 * `IteratorFacade<IterTypes>` as a friend so the facade can call them.
 */
template<typename IterTypes>
struct IteratorFacade {
  using value_type = IterTypes::IterValue;
  using reference = IterTypes::IterRef;
  using pointer = IterTypes::IterPtr;
  using difference_type = IterTypes::IterDiff;

  //------------------------------------------------------------------------------------------------
  // Forward iterator operations
  //------------------------------------------------------------------------------------------------

  /** Dereferences the iterator. */
  constexpr reference operator*(this auto&& self) {
    return self.deref();
  }

  /** Provides member access to the dereferenced value. */
  constexpr auto operator->(this const auto& self)
  requires(requires { self.deref(); })
  {
    return ArrowCreator<value_type, pointer>::create(self.deref());
  }

  /** Advances the iterator by one element. */
  template<typename Derived>
  constexpr Derived& operator++(this Derived& self) {
    self.incr();
    return self;
  }

  /** Advances the iterator by one element and returns a copy of the original iterator. */
  template<typename Derived>
  constexpr Derived operator++(this Derived& self, int) {
    Derived tmp{self};
    self.incr();
    return tmp;
  }

  /** Compares two iterators for equality. */
  template<typename Derived>
  constexpr bool operator==(this const Derived& d1, const Derived& d2) {
    return d1.eq(d2);
  }

  /** Compares the iterator with any other supported type for equality. */
  constexpr bool operator==(this const auto& d1, const auto& d2)
  requires(requires { d1.eq(d2); })
  {
    return d1.eq(d2);
  }

  //------------------------------------------------------------------------------------------------
  // Bidirectional iterator operations
  //------------------------------------------------------------------------------------------------

  /** Moves the iterator back by one element. */
  template<typename Derived>
  constexpr Derived& operator--(this Derived& self)
  requires(requires { self.decr(); })
  {
    self.decr();
    return self;
  }

  /** Moves the iterator back by one element and returns a copy of the original iterator. */
  template<typename Derived>
  constexpr Derived operator--(this Derived& self, int)
  requires(requires { self.decr(); })
  {
    Derived tmp{self};
    self.decr();
    return tmp;
  }

  //------------------------------------------------------------------------------------------------
  // Random-access iterator operations
  //------------------------------------------------------------------------------------------------

  /** Advances the iterator by `n` elements. */
  template<typename Derived>
  constexpr Derived& operator+=(this Derived& self, difference_type n)
  requires(requires { self.iadd(n); })
  {
    self.iadd(n);
    return self;
  }

  /** Returns an iterator advanced by `n` elements. */
  template<std::derived_from<IteratorFacade> Derived>
  friend constexpr Derived operator+(const Derived& self, difference_type n)
  requires(requires(Derived tmp) { tmp.iadd(n); })
  {
    Derived tmp{self};
    tmp.iadd(n);
    return tmp;
  }

  /** Returns an iterator advanced by `n` elements. */
  template<std::derived_from<IteratorFacade> Derived>
  friend constexpr Derived operator+(difference_type n, const Derived& self)
  requires(requires(Derived tmp) { tmp.iadd(n); })
  {
    return self + n;
  }

  /** Moves the iterator back by `n` elements. */
  template<typename Derived>
  constexpr Derived& operator-=(this Derived& self, difference_type n)
  requires(requires { self.iadd(-n); })
  {
    if constexpr (requires { self.isub(n); }) {
      self.isub(n);
    } else {
      self.iadd(-n);
    }
    return self;
  }

  /** Returns an iterator moved back by `n` elements. */
  template<typename Derived>
  constexpr Derived operator-(this const Derived& self, difference_type n)
  requires(requires(Derived tmp) { tmp.iadd(-n); })
  {
    Derived tmp{self};
    if constexpr (requires { tmp.isub(n); }) {
      tmp.isub(n);
    } else {
      tmp.iadd(-n);
    }
    return tmp;
  }

  /** Accesses the element `n` positions away from the iterator. */
  template<typename Derived>
  constexpr decltype(auto) operator[](this const Derived& self, difference_type n)
  requires(
    requires { self.get_item(n); } || requires(Derived tmp) { tmp.iadd(n); })
  {
    if constexpr (requires { self.get_item(n); }) {
      return self.get_item(n);
    } else {
      Derived tmp{self};
      tmp.iadd(n);
      return *tmp;
    }
  }

  /** Computes the distance between two iterators. */
  template<typename Derived>
  constexpr difference_type operator-(this const Derived& d1, const Derived& d2)
  requires(requires {
    { d1.sub(d2) } -> std::convertible_to<difference_type>;
  })
  {
    return d1.sub(d2);
  }

  /** Establishes a strict total order between two iterators. */
  template<typename Derived>
  constexpr std::strong_ordering operator<=>(this const Derived& d1, const Derived& d2)
  requires(requires {
    { d1.three_way(d2) } -> std::convertible_to<std::strong_ordering>;
  })
  {
    return d1.three_way(d2);
  }
};
} // namespace thes

#endif // INCLUDE_THESAUROS_ITERATOR_FACADE_HPP
