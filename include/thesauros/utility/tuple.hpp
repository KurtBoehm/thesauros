#ifndef INCLUDE_THESAUROS_UTILITY_TUPLE_HPP
#define INCLUDE_THESAUROS_UTILITY_TUPLE_HPP

#include <cstddef>
#include <utility>

#include "thesauros/utility/static-ranges/definitions/get-at.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/type-tag.hpp"

namespace thes {
namespace detail {
template<std::size_t tIdx, typename T>
struct TupleLeaf {
  T data;

  bool operator==(const TupleLeaf& other) const = default;
};

template<typename TIdxSeq, typename... Ts>
struct Tuple;
template<std::size_t... tIdxs, typename... Ts>
struct Tuple<std::index_sequence<tIdxs...>, Ts...> : detail::TupleLeaf<tIdxs, Ts>... {
  explicit constexpr Tuple(Ts&&... args)
      : detail::TupleLeaf<tIdxs, Ts>{std::forward<Ts>(args)}... {}

  constexpr Tuple()
  requires(... && std::is_default_constructible_v<Ts>)
      : detail::TupleLeaf<tIdxs, Ts>{Ts{}}... {}

  bool operator==(const Tuple& other) const = default;
};
} // namespace detail

template<typename... Ts>
struct Tuple : public detail::Tuple<std::index_sequence_for<Ts...>, Ts...> {
  using Parent = detail::Tuple<std::index_sequence_for<Ts...>, Ts...>;
  using Parent::Parent;
};
template<typename... Ts>
Tuple(Ts&&...) -> Tuple<Ts...>;

template<std::size_t tIdx, typename T>
TypeTag<T> tuple_element_tag(const detail::TupleLeaf<tIdx, T>&);

template<std::size_t tIdx, typename TTuple>
using TupleElement = decltype(tuple_element_tag<tIdx>(std::declval<TTuple>()))::Type;

namespace star {
template<std::size_t tIndex, typename... Ts>
struct GetAtTrait<tIndex, Tuple<Ts...>> {
  template<typename T>
  static constexpr const T& get_at(const ::thes::detail::TupleLeaf<tIndex, T>& t) {
    return t.data;
  }
  template<typename T>
  static constexpr T& get_at(::thes::detail::TupleLeaf<tIndex, T>& t) {
    return t.data;
  }
};

template<typename... Ts>
struct SizeTrait<Tuple<Ts...>> {
  static constexpr std::size_t value = sizeof...(Ts);
};
} // namespace star
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_TUPLE_HPP
