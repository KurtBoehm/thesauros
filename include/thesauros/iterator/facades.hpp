// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_ITERATOR_FACADES_HPP
#define INCLUDE_THESAUROS_ITERATOR_FACADES_HPP

#include <compare>
#include <concepts>
#include <iterator>
#include <type_traits>

#include "thesauros/iterator/concepts.hpp"
#include "thesauros/utility/arrow-proxy.hpp"

namespace thes {
/**
 * A CRTP base that implements the iterator operators of `Derived` in terms of operations supplied
 * by `Provider`.
 *
 * `Derived` must be at least a forward iterator; bidirectional and random-access support are
 * detected automatically based on which operations `Provider` supplies.
 *
 * `Provider` must always provide the following static member functions, where `State` is
 * `Provider::FacadeState` if defined, or `Derived` otherwise:
 * - `deref(const State&) -> convertible_to<reference>`
 * - `incr(State&)`
 * - `eq(const State&, const State&) -> convertible_to<bool>`
 *
 * For a bidirectional iterator, `Provider` must additionally provide `decr(State&)`.
 *
 * For a random-access iterator, `Provider` must additionally provide:
 * - `iadd(State&, difference_type)`
 * - `sub(const State&, const State&) -> convertible_to<difference_type>`
 * - `three_way(const State&, const State&) -> convertible_to<std::strong_ordering>`
 *
 * The following functions may optionally be provided to replace the default implementations of some
 * operators:
 * - `isub(State&, difference_type)`, used for `operator-=` and `operator-`, which otherwise
 *   default to `iadd(state, -n)`.
 * - `get_item(const State&, difference_type)`, used for `operator[]`, which otherwise defaults
 *   to `iadd` followed by `deref`.
 *
 * @tparam Derived The iterator type deriving from this facade.
 * @tparam Provider The type providing the iterator operations.
 */
// TODO: Consider adding default implementations for the comparison operators if `sub` is available.
template<typename Derived, typename Provider>
struct IteratorFacade {
  using IterTypes = Provider::IterTypes;

  using value_type = IterTypes::IterValue;
  using reference = IterTypes::IterRef;
  using pointer = IterTypes::IterPtr;
  using difference_type = IterTypes::IterDiff;

  static constexpr bool has_state = requires { typename Provider::FacadeState; };

  /** Resolves to `Provider::FacadeState` if it exists, or `Derived` otherwise. */
  template<bool HasState, typename = void>
  struct StateTrait;
  template<typename Dummy>
  struct StateTrait<true, Dummy> {
    using Type = Provider::FacadeState;
  };
  template<typename Dummy>
  struct StateTrait<false, Dummy> {
    using Type = Derived;
  };
  using State = StateTrait<has_state>::Type;

  static_assert(iter_provider::ForwardIterProvider<State, Provider>,
                "The implementation assumes at least a forward iterator!");
  static constexpr bool is_forward_iter = true;
  static constexpr bool is_bidirectional_iter =
    iter_provider::BidirectionalIterProvider<State, Provider>;
  static constexpr bool is_random_access_iter =
    iter_provider::RandomAccessIterProvider<State, Provider>;

  using iterator_category =
    std::conditional_t<is_random_access_iter, std::random_access_iterator_tag,
                       std::conditional_t<is_bidirectional_iter, std::bidirectional_iterator_tag,
                                          std::forward_iterator_tag>>;

  //------------------------------------------------------------------------------------------------
  // Forward iterator operations
  //------------------------------------------------------------------------------------------------

  /** Dereferences the iterator. */
  constexpr friend reference operator*(const Derived& self) {
    return Provider::deref(state(self));
  }
  constexpr friend reference operator*(Derived& self) {
    return Provider::deref(state(self));
  }
  constexpr friend reference operator*(Derived&& self) {
    return Provider::deref(state(std::move(self)));
  }

  /** Provides member access to the dereferenced value. */
  constexpr pointer operator->() const
  requires(!std::same_as<pointer, void>)
  {
    return ArrowCreator<value_type, pointer>::create(Provider::deref(state(derived())));
  }

  /** Advances the iterator by one element. */
  constexpr friend Derived& operator++(Derived& self) {
    Provider::incr(state(self));
    return self;
  }
  constexpr friend Derived operator++(Derived& self, int) {
    Derived tmp{self};
    Provider::incr(state(self));
    return tmp;
  }

  /** Compares two iterators for equality. */
  constexpr friend bool operator==(const Derived& d1, const Derived& d2) {
    return Provider::eq(state(d1), state(d2));
  }

  //------------------------------------------------------------------------------------------------
  // Bidirectional iterator operations
  //------------------------------------------------------------------------------------------------

  /** Moves the iterator back by one element. */
  constexpr friend Derived& operator--(Derived& self)
  requires(iter_provider::Decr<State, Provider>)
  {
    Provider::decr(state(self));
    return self;
  }
  constexpr friend Derived operator--(Derived& self, int)
  requires(iter_provider::Decr<State, Provider>)
  {
    Derived tmp{self};
    Provider::decr(state(self));
    return tmp;
  }

  //------------------------------------------------------------------------------------------------
  // Random-access iterator operations
  //------------------------------------------------------------------------------------------------

  /** Advances the iterator by `n` elements. */
  constexpr friend Derived& operator+=(Derived& self, auto n)
  requires(iter_provider::InPlaceAdd<State, Provider>)
  {
    Provider::iadd(state(self), n);
    return self;
  }
  constexpr friend Derived operator+(const Derived& self, auto n)
  requires(iter_provider::InPlaceAdd<State, Provider>)
  {
    Derived tmp{self};
    Provider::iadd(state(tmp), n);
    return tmp;
  }
  constexpr friend Derived operator+(auto n, const Derived& self)
  requires(iter_provider::InPlaceAdd<State, Provider>)
  {
    return self + n;
  }

  /** Moves the iterator back by `n` elements. */
  constexpr friend Derived& operator-=(Derived& self, auto n)
  requires(iter_provider::InPlaceAdd<State, Provider>)
  {
    if constexpr (iter_provider::InPlaceSub<State, Provider>) {
      Provider::isub(state(self), n);
    } else {
      Provider::iadd(state(self), -n);
    }
    return self;
  }
  constexpr friend Derived operator-(const Derived& self, auto n)
  requires(iter_provider::InPlaceAdd<State, Provider>)
  {
    Derived tmp{self};
    if constexpr (iter_provider::InPlaceSub<State, Provider>) {
      Provider::isub(state(tmp), n);
    } else {
      Provider::iadd(state(tmp), -n);
    }
    return tmp;
  }

  /** Accesses the element `n` positions away from the iterator. */
  constexpr reference operator[](auto n) const
  requires(iter_provider::InPlaceAdd<State, Provider>)
  {
    if constexpr (iter_provider::GetItem<State, Provider>) {
      return Provider::get_item(state(derived()), n);
    } else {
      return *(derived() + n);
    }
  }

  /** Computes the distance between two iterators. */
  constexpr friend difference_type operator-(const Derived& self, const Derived& other)
  requires(iter_provider::Sub<State, Provider>)
  {
    return Provider::sub(state(self), state(other));
  }

  /** Establishes a strict total order between two iterators. */
  friend std::strong_ordering operator<=>(const Derived& d1, const Derived& d2)
  requires(iter_provider::ThreeWayCmp<State, Provider>)
  {
    return Provider::three_way(state(d1), state(d2));
  }

private:
  static constexpr State& state(Derived& d) {
    if constexpr (has_state) {
      return d.state();
    } else {
      return d;
    }
  }
  static constexpr const State& state(const Derived& d) {
    if constexpr (has_state) {
      return d.state();
    } else {
      return d;
    }
  }

  constexpr Derived& derived() {
    return static_cast<Derived&>(*this);
  }
  constexpr const Derived& derived() const {
    return static_cast<const Derived&>(*this);
  }
};

namespace iter_provider {
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

/** The `IterTypes` for an iterator that does not support dereferencing. */
template<typename Diff>
struct VoidTypes {
  using IterValue = void;
  using IterRef = void;
  using IterPtr = void;
  using IterDiff = Diff;
};
} // namespace iter_provider
} // namespace thes

#endif // INCLUDE_THESAUROS_ITERATOR_FACADES_HPP
