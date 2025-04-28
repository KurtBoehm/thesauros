#ifndef INCLUDE_THESAUROS_UTILITY_TYPE_SEQUENCE_OPERATIONS_HPP
#define INCLUDE_THESAUROS_UTILITY_TYPE_SEQUENCE_OPERATIONS_HPP

#include <cstddef>
#include <type_traits>

#include "thesauros/static-ranges.hpp"
#include "thesauros/utility/tuple.hpp"
#include "thesauros/utility/type-sequence/type-sequence.hpp"

namespace thes {
// join

template<AnyTypeSeq... Ts>
struct JoinedTypeSeqTrait;

template<>
struct JoinedTypeSeqTrait<> {
  using Type = TypeSeq<>;
};
template<typename... Ts1, typename... Ts2>
struct JoinedTypeSeqTrait<TypeSeq<Ts1...>, TypeSeq<Ts2...>> {
  using Type = TypeSeq<Ts1..., Ts2...>;
};
template<typename... Ts, AnyTypeSeq... TSeqs>
struct JoinedTypeSeqTrait<TypeSeq<Ts...>, TSeqs...>
    : public JoinedTypeSeqTrait<TypeSeq<Ts...>, typename JoinedTypeSeqTrait<TSeqs...>::Type> {};

template<AnyTypeSeq... TSeqs>
using JoinedTypeSeq = JoinedTypeSeqTrait<TSeqs...>::Type;

template<AnyTypeSeq... TSeqs>
inline constexpr JoinedTypeSeq<TSeqs...> join_type_seqs(TSeqs... /*seqs*/) {
  return {};
}
template<AnyTypeSeq... TSeqs>
requires(sizeof...(TSeqs) > 0)
inline constexpr auto join(TSeqs... seqs) {
  return join_type_seqs(seqs...);
}

// Cartesian product

template<typename T, AnyTypeSeq... TSeqs>
inline constexpr auto cartesian_product(TypeSeq<T> /*seq*/) {
  return TypeSeq<TypeSeq<T>>{};
}
template<typename T, AnyTypeSeq... TSeqs>
inline constexpr auto cartesian_product(TypeSeq<T> /*head*/, TSeqs... tail) {
  return []<AnyTypeSeq... TProds>(TypeSeq<TProds...>) {
    return TypeSeq<typename TProds::template Prepended<T>...>{};
  }(cartesian_product(tail...));
}
template<typename... Ts, AnyTypeSeq... TSeqs>
inline constexpr auto cartesian_product(TypeSeq<Ts...> /*head*/, TSeqs... tail) {
  return join_type_seqs(cartesian_product(TypeSeq<Ts>{}, tail...)...);
}

template<AnyTypeSeq... TSeqs>
using CartesianTypeSeq = decltype(cartesian_product(TSeqs{}...));

// flatten

template<AnyTypeSeq T>
struct FlatTypeSeqTrait;
template<typename T>
requires(!AnyTypeSeq<T>)
struct FlatTypeSeqTrait<TypeSeq<T>> {
  using Type = TypeSeq<T>;
};
template<typename... Ts>
struct FlatTypeSeqTrait<TypeSeq<Ts...>> {
  using Type = JoinedTypeSeq<typename FlatTypeSeqTrait<AsTypeSeq<Ts>>::Type...>;
};

template<AnyTypeSeq TSeq>
using FlatTypeSeq = FlatTypeSeqTrait<TSeq>::Type;

template<AnyTypeSeq TSeq>
inline constexpr FlatTypeSeq<TSeq> flatten(TSeq /*seq*/) {
  return {};
}

// transform

template<AnyTypeSeq TSeq, template<typename> typename TMap>
struct TransformedTypeSeqTrait;
template<typename... Ts, template<typename> typename TMap>
struct TransformedTypeSeqTrait<TypeSeq<Ts...>, TMap> {
  using Type = TypeSeq<TMap<Ts>...>;
};

template<AnyTypeSeq TSeq, template<typename> typename TMap>
using TransformedTypeSeq = TransformedTypeSeqTrait<TSeq, TMap>::Type;

// filter

template<AnyTypeSeq TSeq, template<typename> typename TFilter>
struct FilteredTypeSeqTrait;
template<template<typename> typename TFilter>
struct FilteredTypeSeqTrait<TypeSeq<>, TFilter> {
  using Type = TypeSeq<>;
};
template<typename THead, typename... TTail, template<typename> typename TFilter>
struct FilteredTypeSeqTrait<TypeSeq<THead, TTail...>, TFilter> {
  using Rec = FilteredTypeSeqTrait<TypeSeq<TTail...>, TFilter>::Type;
  using Type =
    std::conditional_t<TFilter<THead>::value, typename Rec::template Prepended<THead>, Rec>;
};

template<AnyTypeSeq TSeq, template<typename> typename TFilter>
using FilteredTypeSeq = FilteredTypeSeqTrait<TSeq, TFilter>::Type;

namespace detail {
template<typename TFun>
struct FunFilterTlax {
  template<typename T>
  struct Filter {
    static constexpr bool value = TFun{}(type_tag<T>);
  };
};
} // namespace detail

inline constexpr auto filter(AnyTypeSeq auto seq, auto filter) {
  return FilteredTypeSeq<decltype(seq), detail::FunFilterTlax<decltype(filter)>::template Filter>{};
}

// index filter

template<std::size_t tIdx, AnyTypeSeq TSeq, auto tIdxSeq>
struct IndexFilteredTypeSeqTrait;
template<std::size_t tIdx, auto tIdxSeq>
struct IndexFilteredTypeSeqTrait<tIdx, TypeSeq<>, tIdxSeq> {
  using Type = TypeSeq<>;
};
template<std::size_t tIdx, typename THead, typename... TTail, auto tIdxSeq>
struct IndexFilteredTypeSeqTrait<tIdx, TypeSeq<THead, TTail...>, tIdxSeq> {
  using Rec = IndexFilteredTypeSeqTrait<tIdx + 1, TypeSeq<TTail...>, tIdxSeq>::Type;
  using Type = std::conditional_t<tIdxSeq | star::contains(index_tag<tIdx>),
                                  typename Rec::template Prepended<THead>, Rec>;
};

template<AnyTypeSeq TSeq, auto tIdxSeq>
using IndexFilteredTypeSeq = IndexFilteredTypeSeqTrait<0, TSeq, tIdxSeq>::Type;

// unique type

template<AnyTypeSeq TSeq>
struct UniqueTypeSeqTrait;
template<>
struct UniqueTypeSeqTrait<TypeSeq<>> {
  using Type = TypeSeq<>;
};
template<typename T, typename... Ts>
struct UniqueTypeSeqTrait<TypeSeq<T, Ts...>> {
  template<typename TOther>
  using Filter = std::bool_constant<!std::is_same_v<T, TOther>>;

  using Type =
    UniqueTypeSeqTrait<FilteredTypeSeq<TypeSeq<Ts...>, Filter>>::Type::template Prepended<T>;
};

template<AnyTypeSeq TSeq>
using UniqueTypeSeq = UniqueTypeSeqTrait<TSeq>::Type;

// tuple types

template<typename TTup>
struct TupleTypeSeqTrait;
template<typename... Ts>
struct TupleTypeSeqTrait<Tuple<Ts...>> {
  using Type = TypeSeq<Ts...>;
};

template<typename TTup>
using TupleTypeSeq = TupleTypeSeqTrait<TTup>::Type;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_TYPE_SEQUENCE_OPERATIONS_HPP
