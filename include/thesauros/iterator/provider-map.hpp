#ifndef INCLUDE_THESAUROS_ITERATOR_PROVIDER_MAP_HPP
#define INCLUDE_THESAUROS_ITERATOR_PROVIDER_MAP_HPP

#include <compare>
#include <concepts>

#include "concepts.hpp"

namespace thes::iter_provider {
// `StateProvider` needs to provide three static member functions:
// * deref(const Derived&) -> convertible_to<StateProvider::value_type>
// * state(Derived&) -> State&
// * state(const Derived&) -> const State&
//
// Additionally, the following static member functions may be provided
// for added functionality:
// * test_if_cmp(const Derived&, const Derived&)
//   * This is called when comparing instances and is intended for assertions.
template<typename TStateProvider, typename TDerived>
struct Map {
  using IterTypes = TStateProvider::IterTypes;
  using Ref = IterTypes::IterRef;
  using Diff = IterTypes::IterDiff;
  using State = IterTypes::IterState;

  static constexpr Ref deref(const auto& self) {
    return TStateProvider::deref(self);
  }

  static constexpr void incr(auto& self)
  requires iter::IncrAny<State>
  {
    ++TStateProvider::state(self);
  }
  static constexpr void decr(auto& self)
  requires iter::DecrAny<State>
  {
    --TStateProvider::state(self);
  }

  static constexpr void iadd(auto& self, Diff d)
  requires iter::InPlaceAdd<State, Diff>
  {
    if constexpr (std::integral<State>) {
      TStateProvider::state(self) += static_cast<State>(d);
    } else {
      TStateProvider::state(self) += d;
    }
  }
  static constexpr void isub(auto& self, Diff d)
  requires iter::InPlaceSub<State, Diff>
  {
    if constexpr (std::integral<State>) {
      TStateProvider::state(self) -= static_cast<State>(d);
    } else {
      TStateProvider::state(self) -= d;
    }
  }

  static constexpr bool eq(const auto& i1, const auto& i2)
  requires iter::Eq<State>
  {
    test_if_cmp(i1, i2);
    return TStateProvider::state(i1) == TStateProvider::state(i2);
  }
  static constexpr std::strong_ordering three_way(const auto& i1, const auto& i2)
  requires iter::ThreeWayCmp<State>
  {
    test_if_cmp(i1, i2);
    return TStateProvider::state(i1) <=> TStateProvider::state(i2);
  }

  static constexpr Diff sub(const auto& i1, const auto& i2)
  requires iter::Sub<State, Diff>
  {
    test_if_cmp(i1, i2);
    if constexpr (std::integral<State>) {
      return static_cast<Diff>(TStateProvider::state(i1)) -
             static_cast<Diff>(TStateProvider::state(i2));
    } else {
      return TStateProvider::state(i1) - TStateProvider::state(i2);
    }
  }

private:
  template<typename TIt1, typename TIt2>
  static constexpr void test_if_cmp(const TIt1& i1, const TIt2& i2) {
    if constexpr (iter_provider::TestIfCmp<TStateProvider, TIt1, TIt2>) {
      TStateProvider::test_if_cmp(i1, i2);
    }
  }
};
} // namespace thes::iter_provider

#endif // INCLUDE_THESAUROS_ITERATOR_PROVIDER_MAP_HPP
