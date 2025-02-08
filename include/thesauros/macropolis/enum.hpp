#ifndef INCLUDE_THESAUROS_MACROPOLIS_ENUM_HPP
#define INCLUDE_THESAUROS_MACROPOLIS_ENUM_HPP

#include <boost/preprocessor.hpp>

#include "thesauros/concepts/type-traits.hpp"
#include "thesauros/macropolis/helpers.hpp"
#include "thesauros/utility/static-ranges.hpp"
#include "thesauros/utility/tuple.hpp"
#include "thesauros/utility/value-tag.hpp"

namespace thes {
template<auto tValue, auto tName, auto tSerialName>
struct EnumValueInfo {
  static constexpr auto value = tValue;
  static constexpr auto name = tName;
  static constexpr auto serial_name = tSerialName;
};

template<auto tName, auto tSerialName, auto tValues>
struct EnumInfoTemplate {
  static constexpr auto name = tName;
  static constexpr auto serial_name = tSerialName;
  static constexpr auto values = tValues;
};

template<typename TEnum>
struct EnumInfo;

template<typename TEnum>
requires(requires(TEnum value) { enum_info_adl(value); })
struct EnumInfo<TEnum> : public decltype(enum_info_adl(std::declval<TEnum>())){};

template<typename T>
concept HasEnumInfo = CompleteType<EnumInfo<T>>;

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
  inline consteval auto enum_info_adl(TYPENAME /*dummy*/) { \
    return ::thes::EnumInfoTemplate<THES_POLIS_NAME_STR_##TYPE, THES_POLIS_SERIAL_NAME_STR_##TYPE, \
                                    ::thes::Tuple{BOOST_PP_LIST_FOR_EACH_I( \
                                      THES_POLIS_ENUM_VALUE_DEF, TYPENAME, LIST)}>{}; \
  }

#define THES_DEFINE_ENUM_IMPL(NAME, UNDERLYING, LIST) \
  THES_DEFINE_ENUM_IMPL_ENUM(NAME, UNDERLYING, LIST) \
  THES_DEFINE_ENUM_IMPL_INFO(NAME, THES_POLIS_NAME(NAME), LIST)

#define THES_DEFINE_ENUM_INFO(NAME, ...) \
  THES_DEFINE_ENUM_IMPL_INFO(NAME, THES_POLIS_NAME(NAME), BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))

#define THES_DEFINE_ENUM(NAME, UNDERLYING, ...) \
  THES_DEFINE_ENUM_IMPL(NAME, UNDERLYING, BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))

template<auto tValue>
requires HasEnumInfo<decltype(tValue)>
inline constexpr auto enum_value_info = [] {
  using Info = EnumInfo<decltype(tValue)>;
  constexpr auto values = Info::values;

  auto impl = [&](auto self, auto idx) {
    auto info = star::get_at<idx>(values);
    if constexpr (info.value == tValue) {
      return info;
    } else {
      return self(self, index_tag<idx + 1>);
    }
  };
  return impl(impl, index_tag<0>);
}();

template<HasEnumInfo T>
inline constexpr auto serial_name_of() {
  return EnumInfo<T>::serial_name;
}
template<auto tValue>
requires HasEnumInfo<decltype(tValue)>
inline constexpr auto serial_name_of() {
  return enum_value_info<tValue>.serial_name;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_MACROPOLIS_ENUM_HPP
