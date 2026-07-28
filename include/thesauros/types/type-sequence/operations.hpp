// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_TYPES_TYPE_SEQUENCE_OPERATIONS_HPP
#define INCLUDE_THESAUROS_TYPES_TYPE_SEQUENCE_OPERATIONS_HPP

#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/types/type-sequence/type-sequence.hpp"

namespace thes {
//==================================================================================================
// Join
//==================================================================================================

/** The concatenation of `Seqs...`, in order. */
template<AnyTypeSeq... Seqs>
struct JoinedTypeSeqTrait;

template<>
struct JoinedTypeSeqTrait<> {
  using Type = TypeSeq<>;
};
template<typename... Ts1, typename... Ts2>
struct JoinedTypeSeqTrait<TypeSeq<Ts1...>, TypeSeq<Ts2...>> {
  using Type = TypeSeq<Ts1..., Ts2...>;
};
template<typename... Ts, AnyTypeSeq... Seqs>
struct JoinedTypeSeqTrait<TypeSeq<Ts...>, Seqs...>
    : public JoinedTypeSeqTrait<TypeSeq<Ts...>, typename JoinedTypeSeqTrait<Seqs...>::Type> {};

template<AnyTypeSeq... Seqs>
using JoinedTypeSeq = JoinedTypeSeqTrait<Seqs...>::Type;

/** Returns the concatenation of `seqs...`, in order. */
template<AnyTypeSeq... Seqs>
constexpr JoinedTypeSeq<Seqs...> join_type_seqs(Seqs... /*seqs*/) {
  return {};
}
/** `join_type_seqs`, requiring at least one sequence to be given. */
template<AnyTypeSeq... Seqs>
requires(sizeof...(Seqs) > 0)
constexpr auto join(Seqs... seqs) {
  return join_type_seqs(seqs...);
}

//==================================================================================================
// Cartesian product
//==================================================================================================

template<typename T, AnyTypeSeq... Seqs>
constexpr auto cartesian_product(TypeSeq<T> /*seq*/) {
  return TypeSeq<TypeSeq<T>>{};
}
template<typename T, AnyTypeSeq... Seqs>
constexpr auto cartesian_product(TypeSeq<T> /*head*/, Seqs... tail) {
  return []<AnyTypeSeq... Prods>(TypeSeq<Prods...>) {
    return TypeSeq<typename Prods::template Prepended<T>...>{};
  }(cartesian_product(tail...));
}
/**
 * Returns the Cartesian product of `head` and `tail...`, i.e. `CartesianTypeSeq<decltype(head),
 * decltype(tail)...>{}`.
 */
template<typename... Ts, AnyTypeSeq... Seqs>
constexpr auto cartesian_product(TypeSeq<Ts...> /*head*/, Seqs... tail) {
  return join_type_seqs(cartesian_product(TypeSeq<Ts>{}, tail...)...);
}

/**
 * The n-ary Cartesian product of `Seqs...`, i.e. a `TypeSeq` of `TypeSeq`s, one per combination
 * of a type from each of `Seqs...`.
 */
template<AnyTypeSeq... Seqs>
using CartesianTypeSeq = decltype(cartesian_product(Seqs{}...));

//==================================================================================================
// Flatten
//==================================================================================================

/** `Seq` with all nested `TypeSeq`s recursively concatenated into a single, flat `TypeSeq`. */
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

template<AnyTypeSeq Seq>
using FlatTypeSeq = FlatTypeSeqTrait<Seq>::Type;

/** Returns `FlatTypeSeq<Seq>{}`, `seq` with all nesting recursively flattened. */
template<AnyTypeSeq Seq>
constexpr FlatTypeSeq<Seq> flatten(Seq /*seq*/) {
  return {};
}

//==================================================================================================
// Callable invocation
//==================================================================================================

namespace detail {
/**
 * Invokes `fun` for the type `T`, additionally passing `args...`, supporting the two calling
 * conventions accepted by `transform`, `reduce` and `filter` (and their `Of`/`By` aliases):
 *
 * - `fun(args..., type_tag<T>)`, taking `T` as a `TypeTag` value; or
 * - `fun.template operator()<T>(args...)`, taking `T` as an explicit template parameter, for
 *   callables that have no use for a runtime `TypeTag` value.
 */
template<typename T>
constexpr decltype(auto) invoke_with_type(auto&& fun, auto&&... args) {
  if constexpr (requires { fun(std::forward<decltype(args)>(args)..., type_tag<T>); }) {
    return fun(std::forward<decltype(args)>(args)..., type_tag<T>);
  } else {
    return fun.template operator()<T>(std::forward<decltype(args)>(args)...);
  }
}
} // namespace detail

//==================================================================================================
// Transform
//==================================================================================================

/** `Seq`, with every type mapped through the alias template `Map`. */
template<AnyTypeSeq Seq, template<typename> typename Map>
struct TransformedTypeSeqTrait;
template<typename... Ts, template<typename> typename Map>
struct TransformedTypeSeqTrait<TypeSeq<Ts...>, Map> {
  using Type = TypeSeq<Map<Ts>...>;
};

template<AnyTypeSeq Seq, template<typename> typename Map>
using TransformedTypeSeq = TransformedTypeSeqTrait<Seq, Map>::Type;

namespace detail {
/**
 * Adapts a stateless callable, given as the type `Fun`, into a `Map`-style alias template by
 * reconstructing an instance via `Fun{}` for every call.
 */
template<typename Fun>
struct TypeMapAdapter {
  template<typename T>
  using Map = decltype(invoke_with_type<T>(Fun{}))::Type;
};
/** Adapts a stateless callable, given as the value `Fun`, into a `Map`-style alias template. */
template<auto Fun>
struct NttpMapAdapter {
  template<typename T>
  using Map = decltype(invoke_with_type<T>(Fun))::Type;
};
} // namespace detail

/**
 * `TransformedTypeSeq<Seq, Map>`, with `Map<T> = decltype(invoke_with_type<T>(Fun{}))::Type`.
 */
template<AnyTypeSeq Seq, typename Fun>
using TransformedTypeSeqOf = TransformedTypeSeq<Seq, detail::TypeMapAdapter<Fun>::template Map>;
/**
 * `TransformedTypeSeq<Seq, Map>`, with `Map<T> = decltype(invoke_with_type<T>(Fun))::Type`.
 *
 * Unlike `TransformedTypeSeqOf`, `Fun` (usually a lambda literal) is given directly as a
 * non-type template parameter, so no `decltype` is needed to name its type.
 */
template<AnyTypeSeq Seq, auto Fun>
using TransformedTypeSeqBy = TransformedTypeSeq<Seq, detail::NttpMapAdapter<Fun>::template Map>;

/**
 * Maps every type `T` in `seq` to the type produced by invoking `fun` for `T` (see
 * `invoke_with_type` for the two supported calling conventions).
 *
 * This is the `std::ranges::transform`-style counterpart to `TransformedTypeSeq`: `fun` must be
 * a stateless callable, as it is default-constructed to be evaluated at the type level.
 */
constexpr auto transform(AnyTypeSeq auto seq, auto fun) {
  return TransformedTypeSeqOf<decltype(seq), decltype(fun)>{};
}
/**
 * Maps every type `T` in `seq` to the type produced by invoking `Fun` for `T` (see
 * `invoke_with_type` for the two supported calling conventions).
 *
 * Unlike `transform(seq, fun)`, `Fun` is given as a template argument rather than a function
 * argument, letting a lambda literal be passed directly, without naming its type via `decltype`,
 * e.g. `transform<[]<typename T>(TypeTag<T>) { return type_tag<T*>; }>(seq)`.
 */
template<auto Fun>
constexpr auto transform(AnyTypeSeq auto seq) {
  return TransformedTypeSeqBy<decltype(seq), Fun>{};
}

//==================================================================================================
// Reduce
//==================================================================================================

/** The left fold of `Seq` into `Init`, repeatedly applying the alias template `Combine`. */
template<AnyTypeSeq Seq, typename Init, template<typename, typename> typename Combine>
struct ReducedTypeSeqTrait;
template<typename Init, template<typename, typename> typename Combine>
struct ReducedTypeSeqTrait<TypeSeq<>, Init, Combine> {
  using Type = Init;
};
template<typename Head, typename... Tail, typename Init,
         template<typename, typename> typename Combine>
struct ReducedTypeSeqTrait<TypeSeq<Head, Tail...>, Init, Combine> {
  using Type = ReducedTypeSeqTrait<TypeSeq<Tail...>, Combine<Init, Head>, Combine>::Type;
};

template<AnyTypeSeq Seq, typename Init, template<typename, typename> typename Combine>
using ReducedTypeSeq = ReducedTypeSeqTrait<Seq, Init, Combine>::Type;

namespace detail {
/**
 * Adapts a stateless callable, given as the type `Fun`, into a `Combine`-style alias template by
 * reconstructing an instance via `Fun{}` for every call.
 */
template<typename Fun>
struct TypeReduceAdapter {
  template<typename Acc, typename T>
  using Combine = decltype(invoke_with_type<T>(Fun{}, Acc{}));
};
/** Adapts a stateless callable, given as the value `Fun`, into a `Combine`-style alias template. */
template<auto Fun>
struct NttpReduceAdapter {
  template<typename Acc, typename T>
  using Combine = decltype(invoke_with_type<T>(Fun, Acc{}));
};
} // namespace detail

/**
 * `ReducedTypeSeq<Seq, Init, Combine>`, with
 * `Combine<Acc, T> = decltype(invoke_with_type<T>(Fun{}, Acc{}))`.
 */
template<AnyTypeSeq Seq, typename Init, typename Fun>
using ReducedTypeSeqOf =
  ReducedTypeSeq<Seq, Init, detail::TypeReduceAdapter<Fun>::template Combine>;
/**
 * `ReducedTypeSeq<Seq, Init, Combine>`, with
 * `Combine<Acc, T> = decltype(invoke_with_type<T>(Fun, Acc{}))`.
 *
 * Unlike `ReducedTypeSeqOf`, `Fun` (usually a lambda literal) is given directly as a non-type
 * template parameter, so no `decltype` is needed to name its type.
 */
template<AnyTypeSeq Seq, typename Init, auto Fun>
using ReducedTypeSeqBy =
  ReducedTypeSeq<Seq, Init, detail::NttpReduceAdapter<Fun>::template Combine>;

/**
 * Left-folds `seq` into `init`, i.e. invokes `fun` for every type `T` with `accumulator` (see
 * `invoke_with_type` for the two supported calling conventions), with `accumulator` initially
 * being `init` and afterwards the previous call’s result.
 *
 * This is the `std::reduce`-style counterpart to `ReducedTypeSeq`: `fun` must be a stateless
 * callable, as it is default-constructed to be evaluated at the type level. As with `transform`,
 * both the accumulator and the result are tags (`TypeTag` or `ValueTag`), so folding into either
 * a type or a compile-time value uses the same mechanism.
 */
constexpr auto reduce(AnyTypeSeq auto seq, auto init, auto fun) {
  return ReducedTypeSeqOf<decltype(seq), decltype(init), decltype(fun)>{};
}
/**
 * Left-folds `seq` into `init`, invoking `Fun` for every type `T` with `accumulator` (see
 * `invoke_with_type` for the two supported calling conventions).
 *
 * Unlike `reduce(seq, init, fun)`, `Fun` is given as a template argument rather than a function
 * argument, letting a lambda literal be passed directly, without naming its type via `decltype`.
 */
template<auto Fun>
constexpr auto reduce(AnyTypeSeq auto seq, auto init) {
  return ReducedTypeSeqBy<decltype(seq), decltype(init), Fun>{};
}

//==================================================================================================
// Filter
//==================================================================================================

/** `Seq`, with only the types kept for which `Filter<T>::value` is `true`. */
template<AnyTypeSeq Seq, template<typename> typename Filter>
struct FilteredTypeSeqTrait;
template<template<typename> typename Filter>
struct FilteredTypeSeqTrait<TypeSeq<>, Filter> {
  using Type = TypeSeq<>;
};
template<typename Head, typename... Tail, template<typename> typename Filter>
struct FilteredTypeSeqTrait<TypeSeq<Head, Tail...>, Filter> {
  using Rec = FilteredTypeSeqTrait<TypeSeq<Tail...>, Filter>::Type;
  using Type = std::conditional_t<Filter<Head>::value, typename Rec::template Prepended<Head>, Rec>;
};

template<AnyTypeSeq Seq, template<typename> typename Filter>
using FilteredTypeSeq = FilteredTypeSeqTrait<Seq, Filter>::Type;

namespace detail {
/**
 * Adapts a stateless callable, given as the type `Fun`, into a `Filter`-style trait by
 * reconstructing an instance via `Fun{}` for every call.
 */
template<typename Fun>
struct TypeFilterAdapter {
  template<typename T>
  struct Filter {
    static constexpr bool value = invoke_with_type<T>(Fun{});
  };
};
/** Adapts a stateless callable, given as the value `Fun`, into a `Filter`-style trait. */
template<auto Fun>
struct NttpFilterAdapter {
  template<typename T>
  struct Filter {
    static constexpr bool value = invoke_with_type<T>(Fun);
  };
};
} // namespace detail

/** `FilteredTypeSeq<Seq, Filter>`, with `Filter<T>::value = invoke_with_type<T>(Fun{})`. */
template<AnyTypeSeq Seq, typename Fun>
using FilteredTypeSeqOf = FilteredTypeSeq<Seq, detail::TypeFilterAdapter<Fun>::template Filter>;
/**
 * `FilteredTypeSeq<Seq, Filter>`, with `Filter<T>::value = invoke_with_type<T>(Fun)`.
 *
 * Unlike `FilteredTypeSeqOf`, `Fun` (usually a lambda literal) is given directly as a non-type
 * template parameter, so no `decltype` is needed to name its type.
 */
template<AnyTypeSeq Seq, auto Fun>
using FilteredTypeSeqBy = FilteredTypeSeq<Seq, detail::NttpFilterAdapter<Fun>::template Filter>;

/**
 * Keeps every type `T` in `seq` for which invoking `fun` for `T` (see `invoke_with_type` for the
 * two supported calling conventions) returns `true`.
 *
 * This is the `std::ranges::filter`-style counterpart to `FilteredTypeSeq`: `fun` must be a
 * stateless callable, as it is default-constructed to be evaluated at the type level.
 */
constexpr auto filter(AnyTypeSeq auto seq, auto fun) {
  return FilteredTypeSeqOf<decltype(seq), decltype(fun)>{};
}
/**
 * Keeps every type `T` in `seq` for which invoking `Fun` for `T` (see `invoke_with_type` for the
 * two supported calling conventions) returns `true`.
 *
 * Unlike `filter(seq, fun)`, `Fun` is given as a template argument rather than a function
 * argument, letting a lambda literal be passed directly, without naming its type via `decltype`.
 */
template<auto Fun>
constexpr auto filter(AnyTypeSeq auto seq) {
  return FilteredTypeSeqBy<decltype(seq), Fun>{};
}

//==================================================================================================
// Index filter
//==================================================================================================

/** The types of `Seq` at the indices contained in `IdxSeq`, indexing from `Idx`. */
template<std::size_t Idx, AnyTypeSeq Seq, auto IdxSeq>
struct IndexFilteredTypeSeqTrait;
template<std::size_t Idx, auto IdxSeq>
struct IndexFilteredTypeSeqTrait<Idx, TypeSeq<>, IdxSeq> {
  using Type = TypeSeq<>;
};
template<std::size_t Idx, typename Head, typename... Tail, auto IdxSeq>
struct IndexFilteredTypeSeqTrait<Idx, TypeSeq<Head, Tail...>, IdxSeq> {
  /** Whether `IdxSeq` contains `Idx`, comparing regardless of the indices’ signedness. */
  static constexpr bool is_kept = []<std::size_t... Idxs>(std::index_sequence<Idxs...>) {
    return (... || std::cmp_equal(IdxSeq[Idxs], Idx));
  }(std::make_index_sequence<IdxSeq.size()>{});

  using Rec = IndexFilteredTypeSeqTrait<Idx + 1, TypeSeq<Tail...>, IdxSeq>::Type;
  using Type = std::conditional_t<is_kept, typename Rec::template Prepended<Head>, Rec>;
};

/** `Seq`, with only the types at the indices contained in `IdxSeq` kept. */
template<AnyTypeSeq Seq, auto IdxSeq>
using IndexFilteredTypeSeq = IndexFilteredTypeSeqTrait<0, Seq, IdxSeq>::Type;

//==================================================================================================
// Unique types
//==================================================================================================

/** `Seq`, with repeated types collapsed to their first occurrence. */
template<AnyTypeSeq Seq>
struct UniqueTypeSeqTrait;
template<>
struct UniqueTypeSeqTrait<TypeSeq<>> {
  using Type = TypeSeq<>;
};
template<typename T, typename... Ts>
struct UniqueTypeSeqTrait<TypeSeq<T, Ts...>> {
  template<typename Other>
  using Filter = std::bool_constant<!std::is_same_v<T, Other>>;

  using Type =
    UniqueTypeSeqTrait<FilteredTypeSeq<TypeSeq<Ts...>, Filter>>::Type::template Prepended<T>;
};

template<AnyTypeSeq Seq>
using UniqueTypeSeq = UniqueTypeSeqTrait<Seq>::Type;
} // namespace thes

#endif // INCLUDE_THESAUROS_TYPES_TYPE_SEQUENCE_OPERATIONS_HPP
