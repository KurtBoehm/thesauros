// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_TYPES_TYPE_SEQUENCE_TYPE_SEQUENCE_HPP
#define INCLUDE_THESAUROS_TYPES_TYPE_SEQUENCE_TYPE_SEQUENCE_HPP

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <variant>

#include "thesauros/types/tuple.hpp"

namespace thes {
//==================================================================================================
// Core type
//==================================================================================================

/** The base of `TypeSeq`, providing `Unique` if `Ts...` is nonempty and consists of one type. */
template<typename... Ts>
struct TypeSeqBase {
  /** Whether `Ts...` is nonempty and consists of a single repeated type. */
  static constexpr bool is_unique = false;
};
template<typename Head, typename... Tail>
requires(... && std::same_as<Head, Tail>)
struct TypeSeqBase<Head, Tail...> {
  static constexpr bool is_unique = true;
  /** The single type shared by all of `Ts...`. */
  using Unique = Head;
};

/** A compile-time sequence of types. */
template<typename... Ts>
struct TypeSeq : public TypeSeqBase<Ts...> {
  /** `Ts...`, as a `Tuple`. */
  using AsTuple = Tuple<Ts...>;
  /** The number of types in the sequence. */
  static constexpr std::size_t size = sizeof...(Ts);

  /** The type at index `Index`. */
  template<std::size_t Index>
  using At = TupleElement<Index, AsTuple>;

  /** Whether `T` occurs in the sequence. */
  template<typename T>
  static constexpr bool contains = (... || std::same_as<T, Ts>);

  /** The sequence with `T` prepended. */
  template<typename T>
  using Prepended = TypeSeq<T, Ts...>;

  /** Returns the sequence obtained by prepending this sequence’s types to `seq`. */
  template<typename... Others>
  constexpr TypeSeq<Others..., Ts...> prepend(TypeSeq<Others...> /*seq*/) const {
    return {};
  }
};

/** Whether `T` is a `TypeSeq` specialization. */
template<typename T>
struct IsTypeSeqTrait : public std::false_type {};
template<typename... Ts>
struct IsTypeSeqTrait<TypeSeq<Ts...>> : public std::true_type {};
/** Matches any `TypeSeq` specialization. */
template<typename T>
concept AnyTypeSeq = IsTypeSeqTrait<T>::value;

/** Whether `seq1` and `seq2` consist of the same types in the same order. */
consteval bool operator==(AnyTypeSeq auto seq1, AnyTypeSeq auto seq2) {
  return std::same_as<decltype(seq1), decltype(seq2)>;
}

//==================================================================================================
// Conversions
//==================================================================================================

namespace detail {
template<typename T>
struct AsTypeSeqTrait {
  using Type = TypeSeq<T>;
};
template<typename... Ts>
struct AsTypeSeqTrait<TypeSeq<Ts...>> {
  using Type = TypeSeq<Ts...>;
};
} // namespace detail
/** `T` itself if it is a `TypeSeq`, else the singleton sequence `TypeSeq<T>`. */
template<typename T>
using AsTypeSeq = detail::AsTypeSeqTrait<T>::Type;

namespace detail {
template<typename T>
struct VariantTypeSeqTrait {
  using Type = TypeSeq<T>;
};
template<typename... Ts>
struct VariantTypeSeqTrait<std::variant<Ts...>> {
  using Type = TypeSeq<typename VariantTypeSeqTrait<Ts>::Type...>;
};
template<typename... Ts>
struct VariantTypeSeqTrait<TypeSeq<Ts...>> {
  using Type = TypeSeq<typename VariantTypeSeqTrait<Ts>::Type...>;
};
} // namespace detail
/**
 * Recursively replaces every `std::variant` in `T` (including nested ones, at any depth inside
 * `TypeSeq`s) with a `TypeSeq` of its (recursively converted) alternatives.
 */
template<typename T>
using VariantTypeSeq = detail::VariantTypeSeqTrait<T>::Type;

namespace detail {
template<template<typename...> typename TupleLike, typename Seq>
struct ToTupleLikeTrait;
template<template<typename...> typename TupleLike, typename... Ts>
struct ToTupleLikeTrait<TupleLike, TypeSeq<Ts...>> {
  using Type = TupleLike<Ts...>;
};
} // namespace detail
/** `Seq`’s types, forwarded as the template arguments of `TupleLike`. */
template<template<typename...> typename TupleLike, typename Seq>
using ToTupleLike = detail::ToTupleLikeTrait<TupleLike, Seq>::Type;
} // namespace thes

#endif // INCLUDE_THESAUROS_TYPES_TYPE_SEQUENCE_TYPE_SEQUENCE_HPP
