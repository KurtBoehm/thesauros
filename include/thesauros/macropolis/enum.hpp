#ifndef INCLUDE_THESAUROS_MACROPOLIS_ENUM_HPP
#define INCLUDE_THESAUROS_MACROPOLIS_ENUM_HPP

#include <tuple>

#include <boost/preprocessor.hpp>

#include "thesauros/macropolis/helpers.hpp"
#include "thesauros/utility/value-tag.hpp"

namespace thes {
template<auto tValue, auto tName, auto tSerialName>
struct EnumValueInfo {
  static constexpr auto value = tValue;
  static constexpr auto name = tName;
  static constexpr auto serial_name = tSerialName;
};

template<typename TEnum>
struct EnumInfo;

#define THES_POLIS_ENUM_DEF_IMPL(VALUE) THES_POLIS_NAME_##VALUE
#define THES_POLIS_ENUM_DEF(REC, _, IDX, VALUE) \
  BOOST_PP_COMMA_IF(IDX) THES_POLIS_ENUM_DEF_IMPL(VALUE)

#define THES_POLIS_ENUM_VALUE_DEF_IMPL(TYPE_NAME, VALUE) \
  ::thes::EnumValueInfo<TYPE_NAME::THES_POLIS_NAME_##VALUE, THES_POLIS_NAME_STR_##VALUE, \
                        THES_POLIS_SERIAL_NAME_STR_##VALUE> {}
#define THES_POLIS_ENUM_VALUE_DEF(REC, TYPE_NAME, IDX, VALUE) \
  BOOST_PP_COMMA_IF(IDX) THES_POLIS_ENUM_VALUE_DEF_IMPL(TYPE_NAME, VALUE)

#define THES_DEFINE_ENUM_IMPL_ENUM(NAME, UNDERLYING, LIST) \
  enum struct THES_POLIS_NAME(NAME) : UNDERLYING { \
    BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_ENUM_DEF, BOOST_PP_EMPTY(), LIST) \
  };

#define THES_DEFINE_ENUM_IMPL_INFO(TYPE, TYPENAME, LIST) \
  /* gcc: global qualification of class name is invalid before ‘{’ token */ \
  template<> \
  struct thes::EnumInfo<TYPENAME> { \
    static constexpr auto name = THES_POLIS_NAME_STR_##TYPE; \
    static constexpr auto serial_name = THES_POLIS_SERIAL_NAME_STR_##TYPE; \
    static constexpr std::tuple values{ \
      BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_ENUM_VALUE_DEF, TYPENAME, LIST)}; \
  };

// Version with namespace

#define THES_DEFINE_ENUM_IMPL(NAMESPACE, NAME, UNDERLYING, LIST) \
  namespace NAMESPACE { \
  THES_DEFINE_ENUM_IMPL_ENUM(NAME, UNDERLYING, LIST) \
  } \
  THES_DEFINE_ENUM_IMPL_INFO(NAME, NAMESPACE::THES_POLIS_NAME(NAME), LIST)

#define THES_DEFINE_ENUM(NAMESPACE, NAME, UNDERLYING, ...) \
  THES_DEFINE_ENUM_IMPL(NAMESPACE, NAME, UNDERLYING, BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))

#define THES_DEFINE_ENUM_INFO(NAMESPACE, NAME, ...) \
  THES_DEFINE_ENUM_IMPL_INFO(NAME, NAMESPACE::THES_POLIS_NAME(NAME), \
                             BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))

// Version without namespace

#define THES_DEFINE_ENUM_SIMPLE_IMPL(NAME, UNDERLYING, LIST) \
  THES_DEFINE_ENUM_IMPL_ENUM(NAME, UNDERLYING, LIST) \
  THES_DEFINE_ENUM_IMPL_INFO(NAME, THES_POLIS_NAME(NAME), LIST)

#define THES_DEFINE_ENUM_SIMPLE(NAME, UNDERLYING, ...) \
  THES_DEFINE_ENUM_SIMPLE_IMPL(NAME, UNDERLYING, BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))

#define THES_DEFINE_ENUM_INFO_SIMPLE(NAME, ...) \
  THES_DEFINE_ENUM_IMPL_INFO(NAME, THES_POLIS_NAME(NAME), BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))

template<auto tValue>
requires(requires { sizeof(EnumInfo<decltype(tValue)>); })
inline constexpr auto enum_value_info = [] {
  using Info = EnumInfo<decltype(tValue)>;
  constexpr auto values = Info::values;

  auto impl = [&](auto self, auto idx) {
    auto info = std::get<idx>(values);
    if constexpr (info.value == tValue) {
      return info;
    } else {
      return self(self, index_tag<idx + 1>);
    }
  };
  return impl(impl, index_tag<0>);
}();

template<typename T>
requires(requires { sizeof(EnumInfo<T>); })
inline constexpr auto serial_name_of() {
  return EnumInfo<T>::serial_name;
}
template<auto tValue>
requires(requires { sizeof(EnumInfo<decltype(tValue)>); })
inline constexpr auto serial_name_of() {
  return enum_value_info<tValue>.serial_name;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_MACROPOLIS_ENUM_HPP
