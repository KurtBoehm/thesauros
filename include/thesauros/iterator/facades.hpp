#ifndef INCLUDE_THESAUROS_ITERATOR_FACADES_HPP
#define INCLUDE_THESAUROS_ITERATOR_FACADES_HPP

#include <compare>
#include <iterator>
#include <type_traits>

#include "thesauros/iterator/concepts.hpp"
#include "thesauros/utility/arrow-proxy.hpp"
#include "thesauros/utility/type-tag.hpp"

namespace thes {
// The iter needs to be a forward iter at least.
// Bi-directional and random-access iters are detected automatically.
//
// In any case (i.e. at least a forward iter), “Provider” needs to provide
// the following static member functions:
// * deref(const Derived&) -> convertible_to<Ref>
// * incr(Derived&)
// * eq(const Derived&, const Derived&) -> convertible_to<bool>
//
// If the iter is bi-directional, “Provider” additionally needs to provide
// the following static member function:
// * decr(Derived&)
//
// If the iter is random-access, “Provider” additionally needs to provide
// the following static member functions:
// * iadd(Derived&, Dif)
// * sub(const Derived&, const Derived&) -> convertible_to<Dif>
// * three_way(const Derived&, const Derived&) -> convertible_to<std::strong_ordering>
//
// The following functions may additionally be provided to replace the default implementations
// of certain operators:
// * isub(Derived&, Dif)
//   * This is used for “operator-=” and “operator-”, which are defined by default through
//     “iadd(d, -n)”, with “Derived& d” and “Dif n”.
// * get_item(const Derived&, Dif)
//   * This is used for “operator[]”, which is defined through “iadd” and “deref”
//     by default.
//
// TODO: Consider adding default implementations for the comparison operators if “sub” is
// available.
template<typename TDerived, typename TProvider>
struct IteratorFacade {
  using IterTypes = TProvider::IterTypes;

  /** @brief The type of value accessed through the iter. */
  using value_type = IterTypes::IterValue;
  /** @brief The type of the reference to a value. */
  using reference = IterTypes::IterRef;
  /** @brief The type of the pointer to a value. */
  using pointer = IterTypes::IterPtr;
  /** @brief The type of the difference between two iters. */
  using difference_type = IterTypes::IterDiff;

  static constexpr bool has_state = requires { typename TProvider::FacadeState; };
  using State = decltype([] {
    if constexpr (has_state) {
      return type_tag<typename TProvider::FacadeState>;
    } else {
      return type_tag<TDerived>;
    }
  }())::Type;

  static_assert(iter_provider::ForwardIterProvider<State, TProvider>,
                "The implementation assumes at least a forward iter!");
  static constexpr bool is_forward_iter = true;
  static constexpr bool is_bidirectional_iter =
    iter_provider::BidirectionalIterProvider<State, TProvider>;
  static constexpr bool is_random_access_iter =
    iter_provider::RandomAccessIterProvider<State, TProvider>;

  using iterator_category =
    std::conditional_t<is_random_access_iter, std::random_access_iterator_tag,
                       std::conditional_t<is_bidirectional_iter, std::bidirectional_iterator_tag,
                                          std::forward_iterator_tag>>;

  // forward iter stuff

  constexpr friend reference operator*(const TDerived& self) {
    return TProvider::deref(state(self));
  }
  constexpr pointer operator->() const
  requires(!std::same_as<pointer, void>)
  {
    return ArrowCreator<value_type, pointer>::create(TProvider::deref(state(derived())));
  }
  constexpr friend TDerived& operator++(TDerived& self) {
    TProvider::incr(state(self));
    return self;
  }
  constexpr friend TDerived operator++(TDerived& self, int) {
    TDerived tmp{self};
    TProvider::incr(state(self));
    return tmp;
  }

  constexpr friend bool operator==(const TDerived& d1, const TDerived& d2) {
    return TProvider::eq(state(d1), state(d2));
  }

  // bi-directional iter stuff

  constexpr friend TDerived& operator--(TDerived& self)
  requires iter_provider::Decr<State, TProvider>
  {
    TProvider::decr(state(self));
    return self;
  }

  constexpr friend TDerived operator--(TDerived& self, int)
  requires iter_provider::Decr<State, TProvider>
  {
    TDerived tmp = self;
    TProvider::decr(state(self));
    return tmp;
  }

  // random-access iter stuff

  constexpr friend TDerived& operator+=(TDerived& self, difference_type n)
  requires iter_provider::InPlaceAdd<State, TProvider>
  {
    TProvider::iadd(state(self), n);
    return self;
  }
  constexpr friend TDerived operator+(const TDerived& self, difference_type n)
  requires iter_provider::InPlaceAdd<State, TProvider>
  {
    TDerived tmp = self;
    TProvider::iadd(state(tmp), n);
    return tmp;
  }
  constexpr friend TDerived operator+(difference_type n, const TDerived& self)
  requires iter_provider::InPlaceAdd<State, TProvider>
  {
    return self + n;
  }

  constexpr friend TDerived& operator-=(TDerived& self, difference_type n)
  requires iter_provider::InPlaceAdd<TDerived, TProvider>
  {
    if constexpr (iter_provider::InPlaceSub<State, TProvider>) {
      TProvider::isub(state(self), n);
    } else {
      TProvider::iadd(state(self), -n);
    }
    return self;
  }

  constexpr friend TDerived operator-(const TDerived& self, difference_type n)
  requires iter_provider::InPlaceAdd<State, TProvider>
  {
    TDerived tmp = self;
    if constexpr (iter_provider::InPlaceSub<State, TProvider>) {
      TProvider::isub(state(tmp), n);
    } else {
      TProvider::iadd(state(tmp), -n);
    }
    return tmp;
  }

  constexpr reference operator[](difference_type n) const
  requires iter_provider::InPlaceAdd<State, TProvider>
  {
    if constexpr (iter_provider::GetItem<State, TProvider>) {
      return TProvider::get_item(state(derived()), n);
    } else {
      return *(derived() + n);
    }
  }

  constexpr friend difference_type operator-(const TDerived& self, const TDerived& other)
  requires iter_provider::Sub<State, TProvider>
  {
    return TProvider::sub(state(self), state(other));
  }

  friend std::strong_ordering operator<=>(const TDerived& d1, const TDerived& d2)
  requires iter_provider::ThreeWayCmp<State, TProvider>
  {
    return TProvider::three_way(state(d1), state(d2));
  }

private:
  constexpr TDerived& derived() {
    return static_cast<TDerived&>(*this);
  }
  constexpr const TDerived& derived() const {
    return static_cast<const TDerived&>(*this);
  }

  static constexpr State& state(TDerived& d) {
    if constexpr (has_state) {
      return d.state();
    } else {
      return d;
    }
  }
  static constexpr const State& state(const TDerived& d) {
    if constexpr (has_state) {
      return d.state();
    } else {
      return d;
    }
  }
};

namespace iter_provider {
template<typename TValue, typename TDiff>
struct DefaultTypes {
  using IterValue = TValue;
  using IterRef = TValue&;
  using IterPtr = TValue*;
  using IterDiff = TDiff;
};

template<typename TValue, typename TDiff>
struct ValueTypes {
  using IterValue = TValue;
  using IterRef = TValue;
  using IterPtr = ArrowProxy<TValue>;
  using IterDiff = TDiff;
};

template<typename TDiff>
struct VoidTypes {
  using IterValue = void;
  using IterRef = void;
  using IterPtr = void;
  using IterDiff = TDiff;
};
} // namespace iter_provider
} // namespace thes

#endif // INCLUDE_THESAUROS_ITERATOR_FACADES_HPP
