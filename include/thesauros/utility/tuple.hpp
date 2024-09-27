#ifndef INCLUDE_THESAUROS_UTILITY_TUPLE_HPP
#define INCLUDE_THESAUROS_UTILITY_TUPLE_HPP

#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/utility/type-tag.hpp"
#include "thesauros/utility/value-tag.hpp"

namespace thes {
namespace detail {
template<std::size_t tIdx, typename T>
struct TupleLeaf {
  T data;

  constexpr friend bool operator==(const TupleLeaf& a, const TupleLeaf& b) {
    return a.data == b.data;
  }
};

template<typename TIdxSeq, typename... Ts>
struct Tuple;
template<std::size_t... tIdxs, typename... Ts>
struct Tuple<std::index_sequence<tIdxs...>, Ts...> : detail::TupleLeaf<tIdxs, Ts>... {
  explicit constexpr Tuple(Ts&&... args)
      : detail::TupleLeaf<tIdxs, Ts>{std::forward<Ts>(args)}... {}

  template<typename... TOthers>
  requires(sizeof...(TOthers) > 0 && sizeof...(TOthers) == sizeof...(Ts) &&
           (... && std::is_constructible_v<Ts, TOthers>))
  explicit constexpr Tuple(TOthers&&... args)
      : detail::TupleLeaf<tIdxs, Ts>{Ts{std::forward<TOthers>(args)}}... {}

  constexpr Tuple()
  requires(... && std::is_default_constructible_v<Ts>)
      : detail::TupleLeaf<tIdxs, Ts>{Ts{}}... {}

  bool operator==(const Tuple& other) const = default;
};

template<std::size_t tIdx, typename T>
static constexpr const T& get_tuple_at(const TupleLeaf<tIdx, T>& t) {
  return t.data;
}
template<std::size_t tIdx, typename T>
static constexpr T& get_tuple_at(TupleLeaf<tIdx, T>& t) {
  return t.data;
}
} // namespace detail

template<typename... Ts>
struct Tuple : public detail::Tuple<std::index_sequence_for<Ts...>, Ts...> {
  using Parent = detail::Tuple<std::index_sequence_for<Ts...>, Ts...>;
  using Parent::Parent;

  static constexpr std::size_t size = sizeof...(Ts);

  template<std::size_t tIdx>
  requires(tIdx < size)
  constexpr decltype(auto) get() const {
    return detail::get_tuple_at<tIdx>(*this);
  }
  template<std::size_t tIdx>
  requires(tIdx < size)
  constexpr decltype(auto) get() {
    return detail::get_tuple_at<tIdx>(*this);
  }
};
template<typename... Ts>
Tuple(Ts&&...) -> Tuple<Ts...>;
template<auto... tValues>
inline constexpr Tuple<AutoTag<tValues>...> tag_tuple{};

template<std::size_t tIdx, typename T>
TypeTag<T> tuple_element_tag(const detail::TupleLeaf<tIdx, T>&);

template<std::size_t tIdx, typename TTuple>
using TupleElement = decltype(tuple_element_tag<tIdx>(std::declval<TTuple>()))::Type;
} // namespace thes

// Add support for structured bindings
namespace std {
template<typename... Ts>
struct tuple_size<::thes::Tuple<Ts...>>
    : public std::integral_constant<std::size_t, sizeof...(Ts)> {};

template<std::size_t tIdx, typename... Ts>
struct tuple_element<tIdx, ::thes::Tuple<Ts...>> {
  using type = ::thes::TupleElement<tIdx, ::thes::Tuple<Ts...>>;
};
} // namespace std

#endif // INCLUDE_THESAUROS_UTILITY_TUPLE_HPP
