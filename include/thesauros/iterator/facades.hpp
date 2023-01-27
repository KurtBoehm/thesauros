#ifndef INCLUDE_THESAUROS_ITERATOR_FACADES_HPP
#define INCLUDE_THESAUROS_ITERATOR_FACADES_HPP

#include <compare>
#include <iterator>
#include <type_traits>

#include "thesauros/iterator/concepts.hpp"
#include "thesauros/utility/arrow-proxy.hpp"
#include "thesauros/utility/type-tag.hpp"

namespace thes {
// The iterator needs to be a forward iterator at least.
// Bi-directional and random-access iterators are detected automatically.
//
// In any case (i.e. at least a forward iterator), “Provider” needs to provide
// the following static member functions:
// * deref(const Derived&) -> convertible_to<Ref>
// * incr(Derived&)
// * eq(const Derived&, const Derived&) -> convertible_to<bool>
//
// If the iterator is bi-directional, “Provider” additionally needs to provide
// the following static member function:
// * decr(Derived&)
//
// If the iterator is random-access, “Provider” additionally needs to provide
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
template<typename Derived, typename Provider>
struct IteratorFacade {
  using IterTypes = typename Provider::IterTypes;

  /** @brief The type of value accessed through the iterator. */
  using value_type = typename IterTypes::IterValue;
  /** @brief The type of the reference to a value. */
  using reference = typename IterTypes::IterRef;
  /** @brief The type of the pointer to a value. */
  using pointer = typename IterTypes::IterPtr;
  /** @brief The type of the difference between two iterators. */
  using difference_type = typename IterTypes::IterDiff;

  static constexpr bool has_state = requires { typename Provider::FacadeState; };
  using State = typename decltype([] {
    if constexpr (has_state) {
      return type_tag<typename Provider::FacadeState>;
    } else {
      return type_tag<Derived>;
    }
  }())::Type;

  static_assert(iterator_provider::ForwardIterProvider<State, Provider>,
                "The implementation assumes at least a forward iterator!");
  static constexpr bool is_forward_iterator = true;
  static constexpr bool is_bidirectional_iterator =
    iterator_provider::BidirectionalIterProvider<State, Provider>;
  static constexpr bool is_random_access_iterator =
    iterator_provider::RandomAccessIterProvider<State, Provider>;

  using iterator_category = std::conditional_t<
    is_random_access_iterator, std::random_access_iterator_tag,
    std::conditional_t<is_bidirectional_iterator, std::bidirectional_iterator_tag,
                       std::forward_iterator_tag>>;

  // forward iterator stuff

  constexpr friend reference operator*(const Derived& self) {
    return Provider::deref(state(self));
  }
  constexpr pointer operator->() const
  requires(!std::same_as<pointer, void>)
  {
    return ArrowCreator<value_type, pointer>::create(**this);
  }
  constexpr friend Derived& operator++(Derived& self) {
    Provider::incr(state(self));
    return self;
  }
  constexpr friend Derived operator++(Derived& self, int) {
    Derived tmp{self};
    Provider::incr(state(self));
    return tmp;
  }

  constexpr friend bool operator==(const Derived& d1, const Derived& d2) {
    return Provider::eq(state(d1), state(d2));
  }

  // bi-directional iterator stuff

  constexpr friend Derived& operator--(Derived& self)
  requires iterator_provider::Decr<State, Provider>
  {
    Provider::decr(state(self));
    return self;
  }

  constexpr friend Derived operator--(Derived& self, int)
  requires iterator_provider::Decr<State, Provider>
  {
    Derived tmp = self;
    Provider::decr(state(self));
    return tmp;
  }

  // random-access iterator stuff

  constexpr friend Derived& operator+=(Derived& self, difference_type n)
  requires iterator_provider::InPlaceAdd<State, Provider>
  {
    Provider::iadd(state(self), n);
    return self;
  }
  constexpr friend Derived operator+(const Derived& self, difference_type n)
  requires iterator_provider::InPlaceAdd<State, Provider>
  {
    Derived tmp = self;
    Provider::iadd(state(tmp), n);
    return tmp;
  }
  constexpr friend Derived operator+(difference_type n, const Derived& self)
  requires iterator_provider::InPlaceAdd<State, Provider>
  {
    return self + n;
  }

  constexpr friend Derived& operator-=(Derived& self, difference_type n)
  requires iterator_provider::InPlaceAdd<Derived, Provider>
  {
    if constexpr (iterator_provider::InPlaceSub<State, Provider>) {
      Provider::isub(state(self), n);
    } else {
      Provider::iadd(state(self), -n);
    }
    return self;
  }

  constexpr friend Derived operator-(const Derived& self, difference_type n)
  requires iterator_provider::InPlaceAdd<State, Provider>
  {
    Derived tmp = self;
    if constexpr (iterator_provider::InPlaceSub<State, Provider>) {
      Provider::isub(state(tmp), n);
    } else {
      Provider::iadd(state(tmp), -n);
    }
    return tmp;
  }

  constexpr reference operator[](difference_type n) const
  requires iterator_provider::InPlaceAdd<State, Provider>
  {
    if constexpr (iterator_provider::GetItem<State, Provider>) {
      return Provider::get_item(state(derived()), n);
    } else {
      return *(derived() + n);
    }
  }

  constexpr friend difference_type operator-(const Derived& self, const Derived& other)
  requires iterator_provider::Sub<State, Provider>
  {
    return Provider::sub(state(self), state(other));
  }

  friend std::strong_ordering operator<=>(const Derived& d1, const Derived& d2)
  requires iterator_provider::ThreeWayCmp<State, Provider>
  {
    return Provider::three_way(state(d1), state(d2));
  }

private:
  constexpr Derived& derived() {
    return static_cast<Derived&>(*this);
  }
  constexpr const Derived& derived() const {
    return static_cast<const Derived&>(*this);
  }

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
};

namespace iterator_provider {
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
} // namespace iterator_provider
} // namespace thes

#endif // INCLUDE_THESAUROS_ITERATOR_FACADES_HPP
