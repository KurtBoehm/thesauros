// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_MACROPOLIS_FLATTEN_TYPE_HPP
#define INCLUDE_THESAUROS_MACROPOLIS_FLATTEN_TYPE_HPP

#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

#include "boost/preprocessor.hpp"

#include "thesauros/static-ranges/definitions.hpp"
#include "thesauros/utility/fancy-visit.hpp"

namespace thes {
namespace impl {
template<typename T1, typename T2>
inline constexpr bool member_ptrs_eq(const T1& /*value1*/, const T2& /*value2*/) {
  return false;
}
template<typename TClass, typename TMember>
inline constexpr bool member_ptrs_eq(TMember TClass::* ptr1, TMember TClass::* ptr2) {
  return ptr1 == ptr2;
}

template<typename TTypeInfo, auto tPtr>
inline constexpr auto member_info_of() {
  constexpr auto members = TTypeInfo::members;

  auto impl = [&]<std::size_t tHead, std::size_t... tTail>(auto rec,
                                                           std::index_sequence<tHead, tTail...>) {
    constexpr auto member = star::get_at<tHead>(members);
    if constexpr (member_ptrs_eq(member.pointer, tPtr)) {
      return member;
    } else {
      return rec(rec, std::index_sequence<tTail...>{});
    }
  };
  return impl(impl, std::make_index_sequence<std::tuple_size_v<decltype(members)>>{});
};
template<typename TTypeInfo, auto tPtr>
using MemberInfoTypeOf = decltype(member_info_of<TTypeInfo, tPtr>());

template<typename T>
struct OptionalVariantTrait {
  using Type = std::variant<std::nullopt_t, T>;
};
template<typename... Ts>
struct OptionalVariantTrait<std::variant<Ts...>> {
  using Type = std::variant<std::nullopt_t, Ts...>;
};
} // namespace impl

template<typename T>
struct FlattenType {
  static constexpr T&& flatten(T&& value) {
    return std::forward<T>(value);
  }
};

template<typename T>
struct FlattenType<std::optional<T>> {
  using Type =
    impl::OptionalVariantTrait<decltype(FlattenType<T>::flatten(std::declval<T&&>()))>::Type;

  static constexpr Type flatten(std::optional<T>&& value) {
    if (value.has_value()) {
      return fancy_visit(
        []<typename TFlat>(TFlat&& flat) {
          return Type{std::in_place_type<TFlat>, std::forward<TFlat>(flat)};
        },
        FlattenType<T>::flatten(std::forward<T>(*value)));
    }
    return Type{std::in_place_type<std::nullopt_t>, std::nullopt};
  }
};

template<typename TVar>
static constexpr auto flatten_variant(TVar&& value) {
  return fancy_flat_visit(
    []<typename T1>(auto maker1, T1&& inner1) {
      return fancy_visit_with_maker(
        maker1,
        []<typename T2>(auto maker2, T2&& inner2) {
          return maker2(std::in_place_type<T2>, std::forward<T2>(inner2));
        },
        FlattenType<std::decay_t<T1>>::flatten(std::forward<T1>(inner1)));
    },
    std::forward<TVar>(value));
}

template<typename... Ts>
struct FlattenType<std::variant<Ts...>> {
  static constexpr auto flatten(std::variant<Ts...>&& value) {
    return flatten_variant(std::move(value));
  }
};

template<typename T>
requires(requires(T&& value) { std::forward<T>(value).flatten(); })
struct FlattenType<T> {
  static constexpr auto flatten(T&& value) {
    return std::forward<T>(value).flatten();
  }
};

template<typename T>
inline constexpr decltype(auto) flatten_type(T&& value) {
  return FlattenType<std::decay_t<T>>::flatten(std::forward<T>(value));
}

#define THES_POLIS_FLATTEN_TYPE_VAR_MEMS_IMPL(TYPE, NAME) &Self::NAME
#define THES_POLIS_FLATTEN_TYPE_VAR_MEMS(REC, TYPENAME, IDX, ELEM) \
  BOOST_PP_COMMA_IF(IDX) \
  THES_POLIS_CALL_TYPED(THES_POLIS_FLATTEN_TYPE_VAR_MEMS_IMPL, ELEM)

#define THES_POLIS_FLATTEN_TYPE_TYPENAME(REC, TYPENAME, IDX, ELEM) \
  BOOST_PP_COMMA_IF(IDX) BOOST_PP_REMOVE_PARENS(ELEM)

#define THES_POLIS_FLATTEN_TYPE_VISITED_TYPES_IMPL(TYPE, NAME) BOOST_PP_REMOVE_PARENS(TYPE)
#define THES_POLIS_FLATTEN_TYPE_VISITED_TYPES(REC, TYPENAME, IDX, ELEM) \
  BOOST_PP_COMMA_IF(IDX) \
  THES_POLIS_CALL_TYPED(THES_POLIS_FLATTEN_TYPE_VISITED_TYPES_IMPL, ELEM)

#define THES_POLIS_FLATTEN_TYPE_VISITED_VALUES_IMPL(TYPE, NAME) \
  std::forward<BOOST_PP_REMOVE_PARENS(TYPE)>(p_##NAME)
#define THES_POLIS_FLATTEN_TYPE_VISITED_VALUES(REC, TYPENAME, IDX, ELEM) \
  BOOST_PP_COMMA_IF(IDX) \
  THES_POLIS_CALL_TYPED(THES_POLIS_FLATTEN_TYPE_VISITED_VALUES_IMPL, ELEM)

// flatten variant lambda template param

#define THES_POLIS_FLATTEN_TYPE_LAMBDA_TYPENAME_IMPL(TYPE, NAME) \
  typename BOOST_PP_REMOVE_PARENS(TYPE)
#define THES_POLIS_FLATTEN_TYPE_LAMBDA_TYPENAME(REC, TYPENAME, IDX, ELEM) \
  BOOST_PP_COMMA_IF(IDX) \
  THES_POLIS_CALL_TYPED(THES_POLIS_FLATTEN_TYPE_LAMBDA_TYPENAME_IMPL, ELEM)

// flatten variants param

#define THES_POLIS_FLATTEN_TYPE_PARAM_IMPL(TYPE, NAME) BOOST_PP_REMOVE_PARENS(TYPE) && p_##NAME
#define THES_POLIS_FLATTEN_TYPE_PARAM(REC, TYPENAME, IDX, ELEM) \
  BOOST_PP_COMMA_IF(IDX) THES_POLIS_CALL_TYPED(THES_POLIS_FLATTEN_TYPE_PARAM_IMPL, ELEM)

// flatten variants forward variant

#define THES_POLIS_FLATTEN_TYPE_VARIANT_IMPL(TYPE, NAME) \
  ::thes::flatten_type( \
    std::forward<typename ::thes::impl::MemberInfoTypeOf<TypeInfo, &Self::NAME>::Type>(NAME))
#define THES_POLIS_FLATTEN_TYPE_VARIANT(REC, TYPENAME, IDX, ELEM) \
  BOOST_PP_COMMA_IF(IDX) \
  THES_POLIS_CALL_TYPED(THES_POLIS_FLATTEN_TYPE_VARIANT_IMPL, ELEM)

#define THES_DEFINE_FLATTEN_TYPE_IMPL(VARIANT_MEMBERS, TEMPLATE_PARAMS) \
  constexpr auto flatten()&& { \
    using Self = TypeInfo::Type; \
    using MemberInfos = TypeInfo::Members; \
    constexpr auto member_infos = TypeInfo::members; \
\
    constexpr std::tuple variant_members{ \
      BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_FLATTEN_TYPE_VAR_MEMS, TYPENAME, VARIANT_MEMBERS)}; \
\
    constexpr auto make_index_sequence = [](const auto& tuple) { \
      return std::make_index_sequence<std::tuple_size_v<std::decay_t<decltype(tuple)>>>{}; \
    }; \
\
    constexpr auto variant_index_of = [=]<std::size_t tIdx>(std::index_sequence<tIdx>) { \
      constexpr auto ptr = ::thes::star::get_at<tIdx>(member_infos).pointer; \
      std::optional<std::size_t> index = std::nullopt; \
      [&]<std::size_t... tIdxs>(std::index_sequence<tIdxs...>) { \
        return ((::thes::impl::member_ptrs_eq(::thes::star::get_at<tIdxs>(variant_members), ptr) \
                   ? (index = tIdxs, true) \
                   : false) || \
                ...); \
      }(make_index_sequence(variant_members)); \
      return index; \
    }; \
\
    return ::thes::fancy_visit( \
      [&]<BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_FLATTEN_TYPE_LAMBDA_TYPENAME, TYPENAME, \
                                   VARIANT_MEMBERS)>( \
        BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_FLATTEN_TYPE_PARAM, TYPENAME, VARIANT_MEMBERS)) { \
        using Out = TypeInfo::template TemplateType<BOOST_PP_LIST_FOR_EACH_I( \
          THES_POLIS_FLATTEN_TYPE_TYPENAME, TYPENAME, TEMPLATE_PARAMS)>; \
\
        using Visited = std::tuple<BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_FLATTEN_TYPE_VISITED_TYPES, \
                                                            TYPENAME, VARIANT_MEMBERS)>; \
        Visited visited{BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_FLATTEN_TYPE_VISITED_VALUES, TYPENAME, \
                                                 VARIANT_MEMBERS)}; \
\
        auto impl = [&]<std::size_t tIdx>(std::index_sequence<tIdx>) -> decltype(auto) { \
          constexpr auto variant_idx = variant_index_of(std::index_sequence<tIdx>{}); \
          if constexpr (variant_idx.has_value()) { \
            return std::forward<std::tuple_element_t<*variant_idx, Visited>>( \
              thes::star::get_at<*variant_idx>(visited)); \
          } else { \
            return std::forward<typename std::tuple_element_t<tIdx, MemberInfos>::Type>( \
              this->*thes::star::get_at<tIdx>(member_infos).pointer); \
          } \
        }; \
\
        return [&]<std::size_t... tIdxs>(std::index_sequence<tIdxs...>) { \
          return Out{impl(std::index_sequence<tIdxs>{})...}; \
        }(make_index_sequence(member_infos)); \
      }, \
      BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_FLATTEN_TYPE_VARIANT, TYPENAME, VARIANT_MEMBERS)); \
  }

#define THES_DEFINE_FLATTEN_TYPE(TEMPLATE_PARAMS, ...) \
  THES_DEFINE_FLATTEN_TYPE_IMPL(BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__), \
                                BOOST_PP_VARIADIC_TO_LIST TEMPLATE_PARAMS)
} // namespace thes

#endif // INCLUDE_THESAUROS_MACROPOLIS_FLATTEN_TYPE_HPP
