#ifndef INCLUDE_THESAUROS_UTILITY_TYPE_SEQUENCE_HPP
#define INCLUDE_THESAUROS_UTILITY_TYPE_SEQUENCE_HPP

#include <concepts>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace thes {
template<typename... Ts>
struct UniqueTrait {
  static constexpr bool is_unique = false;
};

template<typename THead, typename... TTail>
requires(... && std::same_as<THead, TTail>)
struct UniqueTrait<THead, TTail...> {
  static constexpr bool is_unique = true;
  using Unique = THead;
};

template<typename... Ts>
struct TypeSeq : public UniqueTrait<Ts...> {
  template<std::size_t tIndex>
  using GetAt = std::tuple_element_t<tIndex, std::tuple<Ts...>>;

  template<typename T>
  using Prepended = TypeSeq<T, Ts...>;
};

template<typename T>
struct IsTypeSeqTrait : public std::false_type {};
template<typename... Ts>
struct IsTypeSeqTrait<TypeSeq<Ts...>> : public std::true_type {};
template<typename T>
inline constexpr bool is_type_seq = IsTypeSeqTrait<T>::value;

namespace impl::as_type_seq {
template<typename T>
struct Impl {
  using Type = TypeSeq<T>;
};
template<typename... Ts>
struct Impl<TypeSeq<Ts...>> {
  using Type = TypeSeq<Ts...>;
};
} // namespace impl::as_type_seq
template<typename T>
using AsTypeSeq = typename impl::as_type_seq::Impl<T>::Type;

namespace impl::converted_type_seq {
template<typename T>
struct Impl {
  using Type = TypeSeq<T>;
};
template<typename... Ts>
struct Impl<std::variant<Ts...>> {
  using Type = TypeSeq<typename Impl<Ts>::Type...>;
};
template<typename... Ts>
struct Impl<TypeSeq<Ts...>> {
  using Type = TypeSeq<typename Impl<Ts>::Type...>;
};
} // namespace impl::converted_type_seq
template<typename T>
using ConvertedTypeSeq = typename impl::converted_type_seq::Impl<T>::Type;

namespace impl::joined_type_seq {
template<typename... Ts>
struct Impl;

template<>
struct Impl<> {
  using Type = TypeSeq<>;
};

template<typename... Ts1, typename... Ts2>
struct Impl<TypeSeq<Ts1...>, TypeSeq<Ts2...>> {
  using Type = TypeSeq<Ts1..., Ts2...>;
};

template<typename... Ts, typename... TTuples>
struct Impl<TypeSeq<Ts...>, TTuples...>
    : public Impl<TypeSeq<Ts...>, typename Impl<TTuples...>::Type> {};
} // namespace impl::joined_type_seq

template<typename... Ts>
using JoinedTypeSeq = typename impl::joined_type_seq::Impl<Ts...>::Type;

namespace impl::product_type_seq {
template<typename T1, typename T2>
struct Base;
template<typename T1, typename... TTypes2>
struct Base<TypeSeq<T1>, TypeSeq<TTypes2...>> {
  using Type = TypeSeq<thes::JoinedTypeSeq<TypeSeq<T1>, TTypes2>...>;
};
template<typename... TTypes, typename... TTups>
struct Base<TypeSeq<TTypes...>, TypeSeq<TTups...>> {
  using Type = thes::JoinedTypeSeq<typename Base<TypeSeq<TTypes>, TypeSeq<TTups...>>::Type...>;
};

template<typename... TTups>
struct Impl;
template<>
struct Impl<> {
  using Type = TypeSeq<>;
};
template<typename... TTypes>
struct Impl<TypeSeq<TTypes...>> {
  using Type = TypeSeq<TypeSeq<TTypes>...>;
};
template<typename T1, typename... TTypes2>
struct Impl<TypeSeq<T1>, TypeSeq<TTypes2...>> {
  using Type = TypeSeq<TypeSeq<T1, TTypes2>...>;
};
template<typename... TTypes1, typename... TTypes2>
struct Impl<TypeSeq<TTypes1...>, TypeSeq<TTypes2...>> {
  using Type = thes::JoinedTypeSeq<typename Impl<TypeSeq<TTypes1>, TypeSeq<TTypes2...>>::Type...>;
};
template<typename... Ts, typename... TTups>
struct Impl<TypeSeq<Ts...>, TTups...> {
  using Tail = typename Impl<TTups...>::Type;
  using Type = thes::JoinedTypeSeq<typename Base<TypeSeq<Ts>, Tail>::Type...>;
};
} // namespace impl::product_type_seq

template<typename... TTups>
using ProductTypeSeq = typename impl::product_type_seq::Impl<TTups...>::Type;

namespace impl::flat_type_seq {
template<typename T>
struct Impl;

template<typename T>
requires(!is_type_seq<T>)
struct Impl<TypeSeq<T>> {
  using Type = TypeSeq<T>;
};

template<typename... Ts>
struct Impl<TypeSeq<Ts...>> {
  using Type = JoinedTypeSeq<typename Impl<AsTypeSeq<Ts>>::Type...>;
};
} // namespace impl::flat_type_seq

template<typename T>
using FlatTypeSeq = typename impl::flat_type_seq::Impl<T>::Type;

namespace impl {
struct MappedTypeSeq {
  template<typename T, template<typename> typename TMap>
  struct Impl;
  template<typename... Ts, template<typename> typename TMap>
  struct Impl<TypeSeq<Ts...>, TMap> {
    using Type = TypeSeq<TMap<Ts>...>;
  };

  template<typename T, template<typename> typename TMap>
  using Type = typename Impl<T, TMap>::Type;
};
} // namespace impl
template<typename T, template<typename> typename TMap>
using MappedTypeSeq = impl::MappedTypeSeq::Type<T, TMap>;

namespace impl {
struct FilteredTypeSeq {
  template<typename T, template<typename> typename TFilter>
  struct Impl;

  template<typename THead, typename... TTail, template<typename> typename TFilter>
  struct Impl<TypeSeq<THead, TTail...>, TFilter> {
    using Rec = typename Impl<TypeSeq<TTail...>, TFilter>::Type;
    using Type =
      std::conditional_t<TFilter<THead>::value, typename Rec::template Prepended<THead>, Rec>;
  };

  template<template<typename> typename TFilter>
  struct Impl<TypeSeq<>, TFilter> {
    using Type = TypeSeq<>;
  };

  template<typename T, template<typename> typename TFilter>
  using Type = typename Impl<T, TFilter>::Type;
};
} // namespace impl

template<typename T, template<typename> typename TFilter>
using FilteredTypeSeq = impl::FilteredTypeSeq::Type<T, TFilter>;

namespace impl::unique_type_seq {
template<typename T>
struct Impl;

template<typename T, typename... Ts>
struct Impl<TypeSeq<T, Ts...>> {
  template<typename TOther>
  using Filter = std::bool_constant<!std::is_same_v<T, TOther>>;

  using Type =
    typename Impl<thes::FilteredTypeSeq<TypeSeq<Ts...>, Filter>>::Type::template Prepended<T>;
};

template<>
struct Impl<TypeSeq<>> {
  using Type = TypeSeq<>;
};
} // namespace impl::unique_type_seq

template<typename T>
using UniqueTypeSeq = typename impl::unique_type_seq::Impl<T>::Type;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_TYPE_SEQUENCE_HPP
