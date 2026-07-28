// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_TUPLE_HPP
#define INCLUDE_THESAUROS_UTILITY_TUPLE_HPP

#include <array>
#include <compare>
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/types/type-tag.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes {
namespace detail {
template<typename T>
struct IsEqualityComparableTrait : public std::bool_constant<std::equality_comparable<T>> {};
template<typename T, std::size_t S>
struct IsEqualityComparableTrait<std::array<T, S>>
    : public std::bool_constant<std::equality_comparable<T>> {};
template<typename T>
concept EqualityComparable = IsEqualityComparableTrait<std::decay_t<T>>::value;

template<std::size_t I, typename T>
struct TupleLeaf {
  T data;

  constexpr friend bool operator==(const TupleLeaf& a, const TupleLeaf& b)
  requires(EqualityComparable<T>)
  {
    return a.data == b.data;
  }
  constexpr friend auto operator<=>(const TupleLeaf& a, const TupleLeaf& b)
  requires(std::three_way_comparable<T>)
  {
    return a.data <=> b.data;
  }
};

template<typename IdxSeq, typename... Ts>
struct Tuple;
template<std::size_t... Is, typename... Ts>
struct Tuple<std::index_sequence<Is...>, Ts...> : detail::TupleLeaf<Is, Ts>... {
  explicit constexpr Tuple(Ts&&... args) : detail::TupleLeaf<Is, Ts>{std::forward<Ts>(args)}... {}

  template<typename... Vs>
  requires(sizeof...(Vs) > 0 && sizeof...(Vs) == sizeof...(Ts) &&
           (... && std::is_constructible_v<Ts, Vs>))
  explicit constexpr Tuple(Vs&&... args)
      : detail::TupleLeaf<Is, Ts>{Ts{std::forward<Vs>(args)}}... {}

  constexpr Tuple()
  requires(... && std::is_default_constructible_v<Ts>)
      : detail::TupleLeaf<Is, Ts>{Ts{}}... {}

  constexpr bool operator==(const Tuple& other) const = default;
  constexpr auto operator<=>(const Tuple& other) const = default;
};

template<std::size_t I, typename T>
static constexpr const T& get_tuple_at(const TupleLeaf<I, T>& t) {
  return t.data;
}
template<std::size_t I, typename T>
static constexpr T& get_tuple_at(TupleLeaf<I, T>& t) {
  return t.data;
}
} // namespace detail

template<typename... Ts>
struct Tuple : public detail::Tuple<std::index_sequence_for<Ts...>, Ts...> {
  using Parent = detail::Tuple<std::index_sequence_for<Ts...>, Ts...>;
  using Parent::Parent;
  static constexpr std::size_t size = sizeof...(Ts);

  template<std::size_t I>
  requires(I < size)
  friend constexpr decltype(auto) get(const Tuple& self, IndexTag<I> /*index*/ = {}) {
    return detail::get_tuple_at<I>(self);
  }
  template<std::size_t I>
  requires(I < size)
  friend constexpr decltype(auto) get(Tuple& self, IndexTag<I> /*index*/ = {}) {
    return detail::get_tuple_at<I>(self);
  }

  constexpr bool operator==(const Tuple&) const = default;
  constexpr auto operator<=>(const Tuple&) const = default;
};
template<typename... Ts>
Tuple(Ts&&...) -> Tuple<Ts...>;
template<auto... Vs>
inline constexpr Tuple<AutoTag<Vs>...> tag_tuple{};

template<std::size_t I, typename T>
TypeTag<T> tuple_element_tag(const detail::TupleLeaf<I, T>&);

template<std::size_t I, typename Tup>
using TupleElement = decltype(tuple_element_tag<I>(std::declval<Tup>()))::Type;

template<typename... Ts>
constexpr thes::Tuple<Ts...> make_tuple(Ts&&... values) {
  return thes::Tuple<Ts...>{std::forward<Ts>(values)...};
}

template<typename... Ts>
constexpr thes::Tuple<Ts&...> tie(Ts&... values) {
  return thes::Tuple<Ts&...>{values...};
}

template<typename T, typename Idxs>
struct SizedTupleTrait;
template<typename T, std::size_t... I>
struct SizedTupleTrait<T, std::index_sequence<I...>> {
  using Type = Tuple<std::conditional_t<I == 0, T, T>...>;
};
template<typename T, std::size_t S>
using SizedTuple = SizedTupleTrait<T, std::make_index_sequence<S>>::Type;
} // namespace thes

// Add support for structured bindings
namespace std {
template<typename... Ts>
struct tuple_size<::thes::Tuple<Ts...>>
    : public std::integral_constant<std::size_t, sizeof...(Ts)> {};

template<std::size_t I, typename... Ts>
struct tuple_element<I, ::thes::Tuple<Ts...>> {
  using type = ::thes::TupleElement<I, ::thes::Tuple<Ts...>>;
};
} // namespace std

#endif // INCLUDE_THESAUROS_UTILITY_TUPLE_HPP
