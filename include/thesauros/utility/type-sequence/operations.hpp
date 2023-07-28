#ifndef INCLUDE_THESAUROS_UTILITY_TYPE_SEQUENCE_OPERATIONS_HPP
#define INCLUDE_THESAUROS_UTILITY_TYPE_SEQUENCE_OPERATIONS_HPP

#include <cstddef>
#include <type_traits>

#include "thesauros/utility/static-ranges.hpp"
#include "thesauros/utility/type-sequence/type-sequence.hpp"

namespace thes {
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

template<typename T>
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

template<typename T>
using FlatTypeSeq = FlatTypeSeqTrait<T>::Type;

struct TransformedTypeSeqTrait {
  template<typename T, template<typename> typename TMap>
  struct Impl;
  template<typename... Ts, template<typename> typename TMap>
  struct Impl<TypeSeq<Ts...>, TMap> {
    using Type = TypeSeq<TMap<Ts>...>;
  };

  template<typename T, template<typename> typename TMap>
  using Type = Impl<T, TMap>::Type;
};

template<typename T, template<typename> typename TMap>
using TransformedTypeSeq = TransformedTypeSeqTrait::Type<T, TMap>;

struct FilteredTypeSeqTrait {
  template<typename T, template<typename> typename TFilter>
  struct Impl;

  template<typename THead, typename... TTail, template<typename> typename TFilter>
  struct Impl<TypeSeq<THead, TTail...>, TFilter> {
    using Rec = Impl<TypeSeq<TTail...>, TFilter>::Type;
    using Type =
      std::conditional_t<TFilter<THead>::value, typename Rec::template Prepended<THead>, Rec>;
  };

  template<template<typename> typename TFilter>
  struct Impl<TypeSeq<>, TFilter> {
    using Type = TypeSeq<>;
  };

  template<typename T, template<typename> typename TFilter>
  using Type = Impl<T, TFilter>::Type;
};

template<typename T, template<typename> typename TFilter>
using FilteredTypeSeq = FilteredTypeSeqTrait::Type<T, TFilter>;

struct IndexFilteredTypeSeqTrait {
  template<std::size_t tIdx, typename T, auto tIdxSeq>
  struct Impl;

  template<std::size_t tIdx, typename THead, typename... TTail, auto tIdxSeq>
  struct Impl<tIdx, TypeSeq<THead, TTail...>, tIdxSeq> {
    using Rec = Impl<tIdx + 1, TypeSeq<TTail...>, tIdxSeq>::Type;
    using Type = std::conditional_t<star::contains(index_tag<tIdx>)(tIdxSeq),
                                    typename Rec::template Prepended<THead>, Rec>;
  };

  template<std::size_t tIdx, auto tIdxSeq>
  struct Impl<tIdx, TypeSeq<>, tIdxSeq> {
    using Type = TypeSeq<>;
  };

  template<typename T, auto tIdxSeq>
  using Type = Impl<0, T, tIdxSeq>::Type;
};

template<typename T, auto tIdxSeq>
using IndexFilteredTypeSeq = IndexFilteredTypeSeqTrait::Type<T, tIdxSeq>;

template<typename T>
struct UniqueTypeSeqTrait;

template<typename T, typename... Ts>
struct UniqueTypeSeqTrait<TypeSeq<T, Ts...>> {
  template<typename TOther>
  using Filter = std::bool_constant<!std::is_same_v<T, TOther>>;

  using Type =
    UniqueTypeSeqTrait<FilteredTypeSeq<TypeSeq<Ts...>, Filter>>::Type::template Prepended<T>;
};

template<>
struct UniqueTypeSeqTrait<TypeSeq<>> {
  using Type = TypeSeq<>;
};

template<typename T>
using UniqueTypeSeq = UniqueTypeSeqTrait<T>::Type;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_TYPE_SEQUENCE_OPERATIONS_HPP
