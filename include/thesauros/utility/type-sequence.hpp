#ifndef INCLUDE_THESAUROS_UTILITY_TYPE_SEQUENCE_HPP
#define INCLUDE_THESAUROS_UTILITY_TYPE_SEQUENCE_HPP

#include <concepts>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "thesauros/utility/static-ranges/sinks/contains.hpp"
#include "thesauros/utility/static-value.hpp"
#include "thesauros/utility/type-tag.hpp"

namespace thes {
template<typename... Ts>
struct TypeSeqBase {
  static constexpr bool is_unique = false;
};
template<typename THead, typename... TTail>
requires(... && std::same_as<THead, TTail>)
struct TypeSeqBase<THead, TTail...> {
  static constexpr bool is_unique = true;
  using Unique = THead;
};

template<typename... Ts>
struct TypeSeq : public TypeSeqBase<Ts...> {
  using AsTuple = std::tuple<Ts...>;

  template<std::size_t tIndex>
  using GetAt = std::tuple_element_t<tIndex, AsTuple>;

  template<typename T>
  static constexpr bool contains = (... || std::same_as<T, Ts>);

  template<typename T>
  using Prepended = TypeSeq<T, Ts...>;
};

template<typename T>
struct IsTypeSeqTrait : public std::false_type {};
template<typename... Ts>
struct IsTypeSeqTrait<TypeSeq<Ts...>> : public std::true_type {};
template<typename T>
inline constexpr bool is_type_seq = IsTypeSeqTrait<T>::value;

namespace impl {
template<typename T>
struct AsTypeSeq {
  using Type = TypeSeq<T>;
};
template<typename... Ts>
struct AsTypeSeq<TypeSeq<Ts...>> {
  using Type = TypeSeq<Ts...>;
};
} // namespace impl
template<typename T>
using AsTypeSeq = impl::AsTypeSeq<T>::Type;

namespace impl {
template<typename T>
struct ConvertedTypeSeq {
  using Type = TypeSeq<T>;
};
template<typename... Ts>
struct ConvertedTypeSeq<std::variant<Ts...>> {
  using Type = TypeSeq<typename ConvertedTypeSeq<Ts>::Type...>;
};
template<typename... Ts>
struct ConvertedTypeSeq<TypeSeq<Ts...>> {
  using Type = TypeSeq<typename ConvertedTypeSeq<Ts>::Type...>;
};
} // namespace impl
template<typename T>
using ConvertedTypeSeq = impl::ConvertedTypeSeq<T>::Type;

namespace impl {
template<typename... Ts>
struct JoinedTypeSeq;

template<>
struct JoinedTypeSeq<> {
  using Type = TypeSeq<>;
};

template<typename... Ts1, typename... Ts2>
struct JoinedTypeSeq<TypeSeq<Ts1...>, TypeSeq<Ts2...>> {
  using Type = TypeSeq<Ts1..., Ts2...>;
};

template<typename... Ts, typename... TTuples>
struct JoinedTypeSeq<TypeSeq<Ts...>, TTuples...>
    : public JoinedTypeSeq<TypeSeq<Ts...>, typename JoinedTypeSeq<TTuples...>::Type> {};
} // namespace impl

template<typename... Ts>
using JoinedTypeSeq = impl::JoinedTypeSeq<Ts...>::Type;

namespace impl {
template<typename T1, typename T2>
struct ProductTypeSeqBase;
template<typename T1, typename... TTypes2>
struct ProductTypeSeqBase<TypeSeq<T1>, TypeSeq<TTypes2...>> {
  using Type = TypeSeq<thes::JoinedTypeSeq<TypeSeq<T1>, TTypes2>...>;
};
template<typename... TTypes, typename... TTups>
struct ProductTypeSeqBase<TypeSeq<TTypes...>, TypeSeq<TTups...>> {
  using Type =
    thes::JoinedTypeSeq<typename ProductTypeSeqBase<TypeSeq<TTypes>, TypeSeq<TTups...>>::Type...>;
};

template<typename... TTups>
struct ProductTypeSeq;
template<>
struct ProductTypeSeq<> {
  using Type = TypeSeq<>;
};
template<typename... TTypes>
struct ProductTypeSeq<TypeSeq<TTypes...>> {
  using Type = TypeSeq<TypeSeq<TTypes>...>;
};
template<typename T1, typename... TTypes2>
struct ProductTypeSeq<TypeSeq<T1>, TypeSeq<TTypes2...>> {
  using Type = TypeSeq<TypeSeq<T1, TTypes2>...>;
};
template<typename... TTypes1, typename... TTypes2>
struct ProductTypeSeq<TypeSeq<TTypes1...>, TypeSeq<TTypes2...>> {
  using Type =
    thes::JoinedTypeSeq<typename ProductTypeSeq<TypeSeq<TTypes1>, TypeSeq<TTypes2...>>::Type...>;
};
template<typename... Ts, typename... TTups>
struct ProductTypeSeq<TypeSeq<Ts...>, TTups...> {
  using Tail = ProductTypeSeq<TTups...>::Type;
  using Type = thes::JoinedTypeSeq<typename ProductTypeSeqBase<TypeSeq<Ts>, Tail>::Type...>;
};
} // namespace impl

template<typename... TTups>
using ProductTypeSeq = impl::ProductTypeSeq<TTups...>::Type;

namespace impl {
template<typename T>
struct FlatTypeSeq;

template<typename T>
requires(!is_type_seq<T>)
struct FlatTypeSeq<TypeSeq<T>> {
  using Type = TypeSeq<T>;
};

template<typename... Ts>
struct FlatTypeSeq<TypeSeq<Ts...>> {
  using Type = thes::JoinedTypeSeq<typename FlatTypeSeq<thes::AsTypeSeq<Ts>>::Type...>;
};
} // namespace impl

template<typename T>
using FlatTypeSeq = impl::FlatTypeSeq<T>::Type;

namespace impl {
struct TransformedTypeSeq {
  template<typename T, template<typename> typename TMap>
  struct Impl;
  template<typename... Ts, template<typename> typename TMap>
  struct Impl<TypeSeq<Ts...>, TMap> {
    using Type = TypeSeq<TMap<Ts>...>;
  };

  template<typename T, template<typename> typename TMap>
  using Type = Impl<T, TMap>::Type;
};
} // namespace impl
template<typename T, template<typename> typename TMap>
using TransformedTypeSeq = impl::TransformedTypeSeq::Type<T, TMap>;

namespace impl {
struct FilteredTypeSeq {
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
} // namespace impl

template<typename T, template<typename> typename TFilter>
using FilteredTypeSeq = impl::FilteredTypeSeq::Type<T, TFilter>;

namespace impl {
struct IndexFilteredTypeSeq {
  template<std::size_t tIdx, typename T, auto tIdxSeq>
  struct Impl;

  template<std::size_t tIdx, typename THead, typename... TTail, auto tIdxSeq>
  struct Impl<tIdx, TypeSeq<THead, TTail...>, tIdxSeq> {
    using Rec = Impl<tIdx + 1, TypeSeq<TTail...>, tIdxSeq>::Type;
    using Type = std::conditional_t<star::contains(static_auto<tIdx>)(tIdxSeq),
                                    typename Rec::template Prepended<THead>, Rec>;
  };

  template<std::size_t tIdx, auto tIdxSeq>
  struct Impl<tIdx, TypeSeq<>, tIdxSeq> {
    using Type = TypeSeq<>;
  };

  template<typename T, auto tIdxSeq>
  using Type = Impl<0, T, tIdxSeq>::Type;
};
} // namespace impl

template<typename T, auto tIdxSeq>
using IndexFilteredTypeSeq = impl::IndexFilteredTypeSeq::Type<T, tIdxSeq>;

namespace impl {
template<typename T>
struct UniqueTypeSeq;

template<typename T, typename... Ts>
struct UniqueTypeSeq<TypeSeq<T, Ts...>> {
  template<typename TOther>
  using Filter = std::bool_constant<!std::is_same_v<T, TOther>>;

  using Type =
    UniqueTypeSeq<thes::FilteredTypeSeq<TypeSeq<Ts...>, Filter>>::Type::template Prepended<T>;
};

template<>
struct UniqueTypeSeq<TypeSeq<>> {
  using Type = TypeSeq<>;
};
} // namespace impl

template<typename T>
using UniqueTypeSeq = impl::UniqueTypeSeq<T>::Type;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_TYPE_SEQUENCE_HPP
