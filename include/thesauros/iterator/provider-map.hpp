#ifndef INCLUDE_THESAUROS_ITERATOR_PROVIDER_MAP_HPP
#define INCLUDE_THESAUROS_ITERATOR_PROVIDER_MAP_HPP

#include "concepts.hpp"

namespace thes::iterator_provider {
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
  using IterTypes = typename TStateProvider::IterTypes;
  using Ref = typename IterTypes::IterRef;
  using Diff = typename IterTypes::IterDiff;
  using State = typename IterTypes::IterState;

  static constexpr Ref deref(const auto& self) {
    return TStateProvider::deref(self);
  }

  static constexpr void incr(auto& self)
  requires iterator::IncrAny<State>
  {
    ++TStateProvider::state(self);
  }
  static constexpr void decr(auto& self)
  requires iterator::DecrAny<State>
  {
    --TStateProvider::state(self);
  }

  static constexpr void iadd(auto& self, Diff d)
  requires iterator::InPlaceAdd<State, Diff>
  {
    if constexpr (std::integral<State>) {
      TStateProvider::state(self) += static_cast<State>(d);
    } else {
      TStateProvider::state(self) += d;
    }
  }
  static constexpr void isub(auto& self, Diff d)
  requires iterator::InPlaceSub<State, Diff>
  {
    if constexpr (std::integral<State>) {
      TStateProvider::state(self) -= static_cast<State>(d);
    } else {
      TStateProvider::state(self) -= d;
    }
  }

  static constexpr bool eq(const auto& i1, const auto& i2)
  requires iterator::Eq<State>
  {
    test_if_cmp(i1, i2);
    return TStateProvider::state(i1) == TStateProvider::state(i2);
  }
  static constexpr std::strong_ordering three_way(const auto& i1, const auto& i2)
  requires iterator::ThreeWayCmp<State>
  {
    test_if_cmp(i1, i2);
    return TStateProvider::state(i1) <=> TStateProvider::state(i2);
  }

  static constexpr Diff sub(const auto& i1, const auto& i2)
  requires iterator::Sub<State, Diff>
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
  template<typename I1, typename I2>
  static constexpr void test_if_cmp(const I1& i1, const I2& i2) {
    if constexpr (iterator_provider::TestIfCmp<TStateProvider, I1, I2>) {
      TStateProvider::test_if_cmp(i1, i2);
    }
  }
};
} // namespace thes::iterator_provider

#endif // INCLUDE_THESAUROS_ITERATOR_PROVIDER_MAP_HPP
