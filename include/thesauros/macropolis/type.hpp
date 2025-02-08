#ifndef INCLUDE_THESAUROS_MACROPOLIS_TYPE_HPP
#define INCLUDE_THESAUROS_MACROPOLIS_TYPE_HPP

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/concepts/type-traits.hpp"
#include "thesauros/utility/primitives.hpp"
#include "thesauros/utility/static-string/static-string.hpp"
#include "thesauros/utility/tuple.hpp"
#include "thesauros/utility/type-transformations.hpp"

namespace thes {
template<auto tName, auto tSerialName, auto tValue>
struct StaticMemberInfo {
  using Type = std::decay_t<decltype(tValue)>;
  static constexpr auto name = tName;
  static constexpr auto serial_name = tSerialName;
  static constexpr auto value = tValue;
};

template<typename T, auto tName, auto tSerialName, auto tPointer>
struct MemberInfo {
  using Type = T;
  static constexpr auto name = tName;
  static constexpr auto serial_name = tSerialName;
  static constexpr auto pointer = tPointer;

  static_assert(std::same_as<std::remove_cvref_t<T>, MemberType<decltype(tPointer)>>);

  template<typename TOther>
  constexpr decltype(auto) value(TOther&& val) {
    return std::forward<TOther>(val).*pointer;
  }
};

template<typename T, auto tName, auto tPointer, std::size_t tOffset>
struct MemberLayoutInfo {
  using Type = T;
  static constexpr auto name = tName;
  static constexpr auto pointer = tPointer;
  static constexpr auto offset = tOffset;
};

template<typename T>
concept HasMemoryLayoutInfo = requires { typename T::MemoryLayoutInfo; };

template<typename T>
inline constexpr auto memory_layout_info = T::MemoryLayoutInfo::members;

template<typename T, auto tName, auto tSerialName, auto tMembers, auto tStaticMembers>
struct TypeInfoTemplate {
  using Type = T;
  static constexpr auto name = tName;
  static constexpr auto serial_name = tSerialName;

  static constexpr ::thes::Tuple members = tMembers;
  static constexpr ::thes::Tuple static_members = tStaticMembers;

  using Members = decltype(members);
};

template<typename T>
struct TypeInfo;
template<typename T>
requires(requires() { type_info_adl(std::declval<T*>()); })
struct TypeInfo<T> {
  using Type = T;
  using Info = decltype(type_info_adl(std::declval<T*>()));
  using Members = Info::Members;

  static constexpr auto name = Info::name;
  static constexpr auto serial_name = Info::serial_name;
  static constexpr auto static_members = Info::static_members;
  static constexpr auto members = Info::members;
};
template<typename T>
concept HasTypeInfo = CompleteType<TypeInfo<T>>;

template<typename T>
struct SerialNameTrait;
template<typename T>
requires(CompleteType<TypeInfo<T>>)
struct SerialNameTrait<T> {
  static constexpr auto name() {
    return TypeInfo<T>::serial_name;
  }
};
template<typename T>
concept HasSerialName = CompleteType<SerialNameTrait<T>>;

#ifdef THES_POLIS_PRIMITIVE_NAME
#error "THES_POLIS_PRIMITIVE_NAME should not be defined!"
#endif
#define THES_POLIS_PRIMITIVE_NAME(TYPENAME) \
  template<> \
  struct SerialNameTrait<::thes::TYPENAME> { \
    static constexpr auto name() { \
      return ::thes::StaticString{#TYPENAME}; \
    }; \
  };

THES_POLIS_PRIMITIVE_NAME(u8)
THES_POLIS_PRIMITIVE_NAME(u16)
THES_POLIS_PRIMITIVE_NAME(u32)
THES_POLIS_PRIMITIVE_NAME(u64)
THES_POLIS_PRIMITIVE_NAME(u128)
THES_POLIS_PRIMITIVE_NAME(i8)
THES_POLIS_PRIMITIVE_NAME(i16)
THES_POLIS_PRIMITIVE_NAME(i32)
THES_POLIS_PRIMITIVE_NAME(i64)
THES_POLIS_PRIMITIVE_NAME(i128)
THES_POLIS_PRIMITIVE_NAME(f16)
THES_POLIS_PRIMITIVE_NAME(f32)
THES_POLIS_PRIMITIVE_NAME(f64)

#undef THES_POLIS_PRIMITIVE_NAME

template<typename T>
requires(CompleteType<SerialNameTrait<T>>)
inline constexpr auto serial_name_of() {
  return SerialNameTrait<T>::name();
}

#define THES_POLIS_MEMBER_NAME_STR_IMPL(NAME, TYPE, ...) THES_POLIS_NAME_STR_##NAME
#define THES_POLIS_MEMBER_NAME_STR(ARG) THES_POLIS_MEMBER_NAME_STR_IMPL ARG

#define THES_POLIS_MEMBER_SERIAL_NAME_STR_IMPL(NAME, TYPE, ...) THES_POLIS_SERIAL_NAME_STR_##NAME
#define THES_POLIS_MEMBER_SERIAL_NAME_STR(ARG) THES_POLIS_MEMBER_SERIAL_NAME_STR_IMPL ARG

#define THES_POLIS_MEMBER_NAME_IMPL(NAME, TYPE, ...) THES_POLIS_NAME_##NAME
#define THES_POLIS_MEMBER_NAME(ARG) THES_POLIS_MEMBER_NAME_IMPL ARG

#define THES_POLIS_MEMBER_TYPE_IMPL(NAME, TYPE, ...) BOOST_PP_REMOVE_PARENS(TYPE)
#define THES_POLIS_MEMBER_TYPE(ARG) THES_POLIS_MEMBER_TYPE_IMPL ARG

#define THES_POLIS_MEMBER_INIT_IMPL(NAME, TYPE, ...) \
  __VA_OPT__(=) BOOST_PP_REMOVE_PARENS(__VA_ARGS__)
#define THES_POLIS_MEMBER_INIT(ARG) THES_POLIS_MEMBER_INIT_IMPL ARG

#define THES_POLIS_MEMBER_CONSTR_INIT_IMPL(NAME, TYPE, ...) \
  __VA_OPT__(=) BOOST_PP_REMOVE_PARENS(__VA_ARGS__)
#define THES_POLIS_MEMBER_CONSTR_INIT(ARG) THES_POLIS_MEMBER_CONSTR_INIT_IMPL ARG

// A static member declaration

#define THES_POLIS_STATIC_MEMBER_DECL_IMPL(NAME, VALUE) \
  static constexpr auto THES_POLIS_NAME(NAME) = VALUE;
#define THES_POLIS_STATIC_MEMBER_DECL(REC, TYPENAME, ARG) THES_POLIS_STATIC_MEMBER_DECL_IMPL ARG

// Generate the StaticMemberInfos

#define THES_POLIS_STATIC_MEMBER_IMPL(NAME, VALUE) \
  ::thes::StaticMemberInfo<THES_POLIS_NAME_STR(NAME), THES_POLIS_SERIAL_NAME_STR(NAME), VALUE> {}
#define THES_POLIS_STATIC_MEMBER(REC, _, IDX, PAIR) \
  BOOST_PP_COMMA_IF(IDX) THES_POLIS_STATIC_MEMBER_IMPL PAIR

// A member declaration

#define THES_POLIS_MEMBER_DECL(REC, TYPENAME, ARG) \
  THES_POLIS_MEMBER_TYPE(ARG) THES_POLIS_MEMBER_NAME(ARG) THES_POLIS_MEMBER_INIT(ARG);

// Body parts

#define THES_POLIS_BODIES(REC, DUMMY, BODY) BOOST_PP_REMOVE_PARENS(BODY)

// Generate the MemberInfos

#define THES_POLIS_MEMBER_INFO(REC, TYPENAME, IDX, ELEM) \
  BOOST_PP_COMMA_IF(IDX) \
  ::thes::MemberInfo<THES_POLIS_MEMBER_TYPE(ELEM), THES_POLIS_MEMBER_NAME_STR(ELEM), \
                     THES_POLIS_MEMBER_SERIAL_NAME_STR(ELEM), \
                     &TYPENAME::THES_POLIS_MEMBER_NAME(ELEM)> {}

// Generate the template params

#define THES_POLIS_TEMPLATE_PARAM_IMPL(TYPE, NAME) TYPE NAME##Inner
#define THES_POLIS_TEMPLATE_PARAM(REC, TYPENAME, IDX, ELEM) \
  BOOST_PP_COMMA_IF(IDX) THES_POLIS_CALL_TYPED(THES_POLIS_TEMPLATE_PARAM_IMPL, ELEM)

#define THES_POLIS_TEMPLATE_PARAM_VALUE_IMPL(TYPE, NAME) NAME##Inner
#define THES_POLIS_TEMPLATE_PARAM_VALUE(REC, TYPENAME, IDX, ELEM) \
  BOOST_PP_COMMA_IF(IDX) \
  THES_POLIS_CALL_TYPED(THES_POLIS_TEMPLATE_PARAM_VALUE_IMPL, ELEM)

#define THES_POLIS_DEFINE_TEMPLATE_TYPE(TYPENAME, TEMPLATE_PARAMS) \
  template<BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_TEMPLATE_PARAM, TYPENAME, TEMPLATE_PARAMS)> \
  using TemplateType = TYPENAME<BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_TEMPLATE_PARAM_VALUE, \
                                                         TYPENAME, TEMPLATE_PARAMS)>;

#define THES_POLIS_TEMPLATE_SPEC_PARAM_IMPL(TYPE, NAME) TYPE NAME
#define THES_POLIS_TEMPLATE_SPEC_PARAM(REC, TYPENAME, IDX, ELEM) \
  BOOST_PP_COMMA_IF(IDX) THES_POLIS_CALL_TYPED(THES_POLIS_TEMPLATE_SPEC_PARAM_IMPL, ELEM)
#define THES_POLIS_TEMPLATE_SPEC(TYPENAME, TEMPLATE_PARAMS) \
  template<BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_TEMPLATE_SPEC_PARAM, TYPENAME, TEMPLATE_PARAMS)>

#define THES_POLIS_TEMPLATE_SPEC_PARAM_NAME_IMPL(TYPE, NAME) NAME
#define THES_POLIS_TEMPLATE_SPEC_PARAM_NAME(REC, TYPENAME, IDX, ELEM) \
  BOOST_PP_COMMA_IF(IDX) \
  THES_POLIS_CALL_TYPED(THES_POLIS_TEMPLATE_SPEC_PARAM_NAME_IMPL, ELEM)
#define THES_POLIS_TEMPLATE_SPEC_NAMES(TYPENAME, TEMPLATE_PARAMS) \
<BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_TEMPLATE_SPEC_PARAM_NAME, TYPENAME, TEMPLATE_PARAMS)>

// Generate the member offsets

#define THES_POLIS_MEMBER_OFFSET(REC, TYPENAME, IDX, ELEM) \
  BOOST_PP_COMMA_IF(IDX) \
  ::thes::MemberLayoutInfo<THES_POLIS_MEMBER_TYPE(ELEM), THES_POLIS_MEMBER_NAME_STR(ELEM), \
                           &TYPENAME::THES_POLIS_MEMBER_NAME(ELEM), \
                           offsetof(TYPENAME, THES_POLIS_MEMBER_NAME(ELEM))> {}

// The constructor implementation

#define THES_POLIS_CONSTRUCTOR_PARAMS(REC, TYPENAME, IDX, ARG) \
  BOOST_PP_COMMA_IF(IDX) \
  THES_POLIS_MEMBER_TYPE(ARG) \
  BOOST_PP_CAT(p_, THES_POLIS_MEMBER_NAME(ARG)) THES_POLIS_MEMBER_CONSTR_INIT(ARG)

#define THES_POLIS_CONSTRUCTOR_INIT_LIST(REC, TYPENAME, IDX, ARG) \
  BOOST_PP_COMMA_IF(IDX) \
  THES_POLIS_MEMBER_NAME(ARG) { \
    std::move(BOOST_PP_CAT(p_, THES_POLIS_MEMBER_NAME(ARG))) \
  }

#define THES_POLIS_CONSTRUCTOR_DEF_IMPL(TYPENAME, LIST)  : BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_CONSTRUCTOR_INIT_LIST, TYPENAME, LIST)
#define THES_POLIS_CONSTRUCTOR_DEF(TYPENAME, LIST) \
  TYPENAME(BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_CONSTRUCTOR_PARAMS, TYPENAME, LIST)) \
  BOOST_PP_REMOVE_PARENS(BOOST_PP_IIF(BOOST_PP_LIST_IS_NIL(LIST), (), \
                                      (THES_POLIS_CONSTRUCTOR_DEF_IMPL(TYPENAME, LIST)))) {}

// Constructor case distinction

#define THES_POLIS_TYPE_CONSTEXPR_CONSTRUCTOR(TYPENAME, LIST) \
  explicit constexpr THES_POLIS_CONSTRUCTOR_DEF(TYPENAME, LIST)
#define THES_POLIS_TYPE_NORMAL_CONSTRUCTOR(TYPENAME, LIST) \
  explicit THES_POLIS_CONSTRUCTOR_DEF(TYPENAME, LIST)
#define THES_POLIS_TYPE_NO_CONSTRUCTOR(TYPENAME, LIST)
#define THES_POLIS_CONSTRUCTOR(TYPENAME, CONSTRUCTOR, LIST) \
  THES_POLIS_TYPE_##CONSTRUCTOR(TYPENAME, LIST)

// Split definitions

#define THES_POLIS_STATIC_MEMBERS_FILTER_PRED_STATIC_MEMBERS(...) 1
#define THES_POLIS_STATIC_MEMBERS_FILTER_PRED_MEMBERS(...) 0
#define THES_POLIS_STATIC_MEMBERS_FILTER_PRED_TEMPLATE_PARAMS(...) 0
#define THES_POLIS_STATIC_MEMBERS_FILTER_PRED_BODY(...) 0

#define THES_POLIS_MEMBERS_FILTER_PRED_STATIC_MEMBERS(...) 0
#define THES_POLIS_MEMBERS_FILTER_PRED_MEMBERS(...) 1
#define THES_POLIS_MEMBERS_FILTER_PRED_TEMPLATE_PARAMS(...) 0
#define THES_POLIS_MEMBERS_FILTER_PRED_BODY(...) 0

#define THES_POLIS_TEMPLATE_PARAMS_FILTER_PRED_STATIC_MEMBERS(...) 0
#define THES_POLIS_TEMPLATE_PARAMS_FILTER_PRED_MEMBERS(...) 0
#define THES_POLIS_TEMPLATE_PARAMS_FILTER_PRED_TEMPLATE_PARAMS(...) 1
#define THES_POLIS_TEMPLATE_PARAMS_FILTER_PRED_BODY(...) 0

#define THES_POLIS_BODY_FILTER_PRED_STATIC_MEMBERS(...) 0
#define THES_POLIS_BODY_FILTER_PRED_MEMBERS(...) 0
#define THES_POLIS_BODY_FILTER_PRED_TEMPLATE_PARAMS(...) 0
#define THES_POLIS_BODY_FILTER_PRED_BODY(...) 1

#define THES_POLIS_ANY_MEMBERS_FILTER_OP_STATIC_MEMBERS(...) __VA_ARGS__
#define THES_POLIS_ANY_MEMBERS_FILTER_OP_MEMBERS(...) __VA_ARGS__
#define THES_POLIS_ANY_MEMBERS_FILTER_OP_TEMPLATE_PARAMS(...) __VA_ARGS__
#define THES_POLIS_ANY_MEMBERS_FILTER_OP_BODY(...) __VA_ARGS__

#define THES_POLIS_ANY_MEMBERS_FILTER_PRED(REC, KIND, ELEM) THES_POLIS_##KIND##_FILTER_PRED_##ELEM
#define THES_POLIS_ANY_MEMBERS_FILTER_OP_IMPL(ELEM) THES_POLIS_ANY_MEMBERS_FILTER_OP_##ELEM
#define THES_POLIS_ANY_MEMBERS_FILTER_OP(REC, KIND, IDX, ELEM) \
  BOOST_PP_COMMA_IF(IDX) THES_POLIS_ANY_MEMBERS_FILTER_OP_IMPL(ELEM)

#define THES_POLIS_ANY_MEMBERS_FILTER_UNPACK(...) BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__)
#define THES_POLIS_ANY_MEMBERS_FILTER_IMPL(KIND, LIST) \
  THES_POLIS_ANY_MEMBERS_FILTER_UNPACK( \
    BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_ANY_MEMBERS_FILTER_OP, KIND, LIST))
#define THES_POLIS_ANY_MEMBERS_FILTER(KIND, LIST) \
  THES_POLIS_ANY_MEMBERS_FILTER_IMPL( \
    KIND, BOOST_PP_LIST_FILTER(THES_POLIS_ANY_MEMBERS_FILTER_PRED, KIND, LIST))

// The main definitions

#define THES_APPLY(MACRO, ...) MACRO(__VA_ARGS__)

#define THES_DEFINE_TYPE_IMPL(TYPE, TYPENAME, CONSTRUCTOR, TEMPLATE_PARAMS, STATIC_MEMBERS, \
                              MEMBERS, BODIES) \
  BOOST_PP_LIST_FOR_EACH(THES_POLIS_STATIC_MEMBER_DECL, TYPENAME, STATIC_MEMBERS) \
  THES_POLIS_CONSTRUCTOR(TYPENAME, CONSTRUCTOR, MEMBERS) \
  BOOST_PP_LIST_FOR_EACH(THES_POLIS_MEMBER_DECL, TYPENAME, MEMBERS) \
  struct TypeInfo { \
    using Type = TYPENAME; \
    BOOST_PP_REMOVE_PARENS(BOOST_PP_IIF(BOOST_PP_LIST_IS_NIL(TEMPLATE_PARAMS), (), \
                                        (THES_POLIS_DEFINE_TEMPLATE_TYPE(TYPENAME, \
                                                                         TEMPLATE_PARAMS)))) \
    static constexpr auto name = THES_POLIS_NAME_STR(TYPE); \
    static constexpr auto serial_name = THES_POLIS_SERIAL_NAME_STR(TYPE); \
\
    static constexpr ::thes::Tuple members{ \
      BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_MEMBER_INFO, TYPENAME, MEMBERS)}; \
    static constexpr ::thes::Tuple static_members{ \
      BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_STATIC_MEMBER, BOOST_PP_EMPTY(), STATIC_MEMBERS)}; \
\
    using Members = decltype(members); \
  }; \
  friend consteval auto type_info_adl(TYPENAME*) { \
    return TypeInfo{}; \
  } \
  BOOST_PP_LIST_FOR_EACH(THES_POLIS_BODIES, TYPENAME, BODIES)

#define THES_CREATE_TYPE_IMPL_INNER(TYPE, TYPENAME, FULL_NAME, CONSTRUCTOR, TEMPLATE_PARAMS, \
                                    STATIC_MEMBERS, MEMBERS, BODIES) \
  BOOST_PP_REMOVE_PARENS(BOOST_PP_IIF(BOOST_PP_LIST_IS_NIL(TEMPLATE_PARAMS), (), \
                                      (THES_POLIS_TEMPLATE_SPEC(TYPENAME, TEMPLATE_PARAMS)))) \
  struct TYPENAME { \
    struct MemoryLayoutInfo; \
    THES_DEFINE_TYPE_IMPL(TYPE, TYPENAME, CONSTRUCTOR, TEMPLATE_PARAMS, STATIC_MEMBERS, MEMBERS, \
                          BODIES) \
  }; \
\
  BOOST_PP_REMOVE_PARENS(BOOST_PP_IIF(BOOST_PP_LIST_IS_NIL(TEMPLATE_PARAMS), (), \
                                      (THES_POLIS_TEMPLATE_SPEC(TYPENAME, TEMPLATE_PARAMS)))) \
  struct BOOST_PP_REMOVE_PARENS(FULL_NAME)::MemoryLayoutInfo { \
    using Self = BOOST_PP_REMOVE_PARENS(FULL_NAME); \
    static constexpr ::thes::Tuple members{ \
      BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_MEMBER_OFFSET, Self, MEMBERS)}; \
  };

#define THES_CREATE_TYPE_IMPL(TYPE, TYPENAME, CONSTRUCTOR, TEMPLATE_PARAMS, STATIC_MEMBERS, \
                              MEMBERS, BODIES) \
  THES_CREATE_TYPE_IMPL_INNER(TYPE, TYPENAME, \
                              (TYPENAME BOOST_PP_REMOVE_PARENS(BOOST_PP_IIF( \
                                BOOST_PP_LIST_IS_NIL(TEMPLATE_PARAMS), (), \
                                (THES_POLIS_TEMPLATE_SPEC_NAMES(TYPENAME, TEMPLATE_PARAMS))))), \
                              CONSTRUCTOR, TEMPLATE_PARAMS, STATIC_MEMBERS, MEMBERS, BODIES)

#define THES_DEFINE_TYPE(TYPE, CONSTRUCTOR, ...) \
  THES_DEFINE_TYPE_IMPL(TYPE, THES_POLIS_NAME(TYPE), CONSTRUCTOR, BOOST_PP_LIST_NIL, \
                        BOOST_PP_LIST_NIL, BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__), \
                        BOOST_PP_LIST_NIL)

#define THES_CREATE_TYPE(TYPE, CONSTRUCTOR, ...) \
  THES_CREATE_TYPE_IMPL(TYPE, THES_POLIS_NAME(TYPE), CONSTRUCTOR, BOOST_PP_LIST_NIL, \
                        BOOST_PP_LIST_NIL, BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__), \
                        BOOST_PP_LIST_NIL)

#define THES_DEFINE_TYPE_EX_IMPL(IMPL, TYPE, CONSTRUCTOR, LIST) \
  THES_APPLY(IMPL, TYPE, THES_POLIS_NAME(TYPE), CONSTRUCTOR, \
             THES_POLIS_ANY_MEMBERS_FILTER(TEMPLATE_PARAMS, LIST), \
             THES_POLIS_ANY_MEMBERS_FILTER(STATIC_MEMBERS, LIST), \
             THES_POLIS_ANY_MEMBERS_FILTER(MEMBERS, LIST), \
             THES_POLIS_ANY_MEMBERS_FILTER(BODY, LIST))

#define THES_DEFINE_TYPE_EX(TYPE, CONSTRUCTOR, ...) \
  THES_DEFINE_TYPE_EX_IMPL(THES_DEFINE_TYPE_IMPL, TYPE, CONSTRUCTOR, \
                           BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))

#define THES_CREATE_TYPE_EX(TYPE, CONSTRUCTOR, ...) \
  THES_DEFINE_TYPE_EX_IMPL(THES_CREATE_TYPE_IMPL, TYPE, CONSTRUCTOR, \
                           BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))

#define THES_POLIS_DEFINE_TEMPLATE_TYPE_INFO(TYPENAME, TEMPLATE_PARAMS) \
  template<BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_TEMPLATE_PARAM, TYPENAME, TEMPLATE_PARAMS)> \
  using TemplateType = TYPENAME<BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_TEMPLATE_PARAM_VALUE, \
                                                         TYPENAME, TEMPLATE_PARAMS)>;
#define THES_DEFINE_TYPE_INFO_IMPL_INNER(TYPE, TYPENAME, TEMPLATE_PARAMS, STATIC_MEMBERS, MEMBERS) \
  BOOST_PP_REMOVE_PARENS(BOOST_PP_IIF( \
    BOOST_PP_LIST_IS_NIL(TEMPLATE_PARAMS), \
    (inline consteval auto type_info_adl(TYPENAME* /*dummy*/) { \
      return ::thes::TypeInfoTemplate< \
        TYPENAME, THES_POLIS_NAME_STR(TYPE), THES_POLIS_SERIAL_NAME_STR(TYPE), \
        ::thes::Tuple{BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_MEMBER_INFO, TYPENAME, MEMBERS)}, \
        ::thes::Tuple{BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_STATIC_MEMBER, BOOST_PP_EMPTY(), \
                                               STATIC_MEMBERS)}>{}; \
    }), \
    (THES_POLIS_DEFINE_TEMPLATE_TYPE_INFO(TYPENAME, TEMPLATE_PARAMS))))

#define THES_DEFINE_TYPE_INFO_IMPL(TYPE, LIST) \
  THES_DEFINE_TYPE_INFO_IMPL_INNER(TYPE, THES_POLIS_NAME(TYPE), \
                                   THES_POLIS_ANY_MEMBERS_FILTER(TEMPLATE_PARAMS, LIST), \
                                   THES_POLIS_ANY_MEMBERS_FILTER(STATIC_MEMBERS, LIST), \
                                   THES_POLIS_ANY_MEMBERS_FILTER(MEMBERS, LIST))
#define THES_DEFINE_TYPE_INFO(TYPE, ...) \
  THES_DEFINE_TYPE_INFO_IMPL(TYPE, BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))
} // namespace thes

#endif // INCLUDE_THESAUROS_MACROPOLIS_TYPE_HPP
