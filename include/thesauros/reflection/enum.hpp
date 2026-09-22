// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_REFLECTION_ENUM_HPP
#define INCLUDE_THESAUROS_REFLECTION_ENUM_HPP

#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <string_view>

#include <boost/preprocessor.hpp>

#include "thesauros/concepts/type-traits.hpp"
#include "thesauros/reflection/helpers.hpp"
#include "thesauros/static-ranges/definitions/get-at.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/static-ranges/definitions/static-apply.hpp"
#include "thesauros/string/static-string.hpp" // IWYU pragma: keep
#include "thesauros/types/tuple.hpp" // IWYU pragma: keep
#include "thesauros/types/type-tag.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes::reflect {
template<auto Value, auto Name, auto SerialName>
struct EnumValueInfo {
  static constexpr auto value = Value;
  static constexpr auto name = Name;
  static constexpr auto serial_name = SerialName;
};

template<auto Name, auto SerialName, auto Values>
struct EnumInfoTemplate {
  static constexpr auto name = Name;
  static constexpr auto serial_name = SerialName;
  static constexpr auto values = Values;
};

template<typename Enum>
struct EnumInfo;

template<typename Enum>
requires(requires(Enum value) { enum_info_adl(value); })
struct EnumInfo<Enum> : decltype(enum_info_adl(std::declval<Enum>())){};

template<typename T>
concept HasEnumInfo = CompleteType<EnumInfo<T>>;

#define THES_POLIS_ENUM_DEF_IMPL(VALUE) THES_POLIS_NAME_##VALUE
#define THES_POLIS_ENUM_DEF(REC, _, IDX, VALUE) \
  BOOST_PP_COMMA_IF(IDX) THES_POLIS_ENUM_DEF_IMPL(VALUE)

#define THES_POLIS_ENUM_VALUE_DEF_IMPL(TYPE_NAME, VALUE) \
  ::thes::reflect::EnumValueInfo<TYPE_NAME::THES_POLIS_NAME_##VALUE, THES_POLIS_NAME_STR_##VALUE, \
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
    return ::thes::reflect::EnumInfoTemplate< \
      THES_POLIS_NAME_STR_##TYPE, THES_POLIS_SERIAL_NAME_STR_##TYPE, \
      ::thes::Tuple{BOOST_PP_LIST_FOR_EACH_I(THES_POLIS_ENUM_VALUE_DEF, TYPENAME, LIST)}>{}; \
  }

#define THES_DEFINE_ENUM_IMPL(NAME, UNDERLYING, LIST) \
  THES_DEFINE_ENUM_IMPL_ENUM(NAME, UNDERLYING, LIST) \
  THES_DEFINE_ENUM_IMPL_INFO(NAME, THES_POLIS_NAME(NAME), LIST)

#define THES_DEFINE_ENUM_INFO(NAME, ...) \
  THES_DEFINE_ENUM_IMPL_INFO(NAME, THES_POLIS_NAME(NAME), BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))

#define THES_DEFINE_ENUM(NAME, UNDERLYING, ...) \
  THES_DEFINE_ENUM_IMPL(NAME, UNDERLYING, BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))

template<auto Value>
requires(HasEnumInfo<decltype(Value)>)
inline constexpr auto enum_value_info = [] {
  // NOLINTNEXTLINE(*-avoid-c-style-cast)
  using Info = EnumInfo<decltype(Value)>;
  constexpr auto values = Info::values;

  return [&](this auto&& self, auto idx) {
    const auto info = star::get_at<idx>(values);
    // NOLINTNEXTLINE(*-avoid-c-style-cast)
    if constexpr (info.value == Value) {
      return info;
    } else {
      return self(index_tag<idx + 1>);
    }
  }(index_tag<0>);
}();

template<HasEnumInfo T>
constexpr auto serial_name_of() {
  return EnumInfo<T>::serial_name;
}
template<auto Value>
requires(HasEnumInfo<decltype(Value)>)
constexpr auto serial_name_of() {
  return enum_value_info<Value>.serial_name;
}

/**
 * The serial name of the enumerator `value` stands for, or `std::nullopt` if it stands for none of
 * them. This is the run-time counterpart of `serial_name_of<Value>()` and the inverse of
 * `enum_cast`, and yields a view rather than a `StaticString` because the length is not known until
 * the enumerator is.
 */
template<HasEnumInfo Enum>
constexpr std::optional<std::string_view> serial_name_of(Enum value) {
  constexpr auto values = EnumInfo<Enum>::values;
  constexpr std::size_t value_num = star::size<decltype(values)>;
  return [&](this auto&& rec, AnyIndexTag auto depth) -> std::optional<std::string_view> {
    constexpr auto value_info = star::get_at<depth>(values);
    if (value_info.value == value) {
      return value_info.serial_name.view();
    }
    if constexpr (depth + 1 < value_num) {
      return rec(index_tag<depth + 1>);
    } else {
      return std::nullopt;
    }
  }(index_tag<0>);
}

// In contrast to magic_enum, this uses the serial names, which seems more appropriate
template<HasEnumInfo T>
constexpr std::optional<T> enum_cast(std::string_view serial_name) {
  constexpr auto values = EnumInfo<T>::values;
  constexpr std::size_t value_num = star::size<decltype(values)>;
  return [&](this auto&& rec, AnyIndexTag auto depth) -> std::optional<T> {
    constexpr auto value_info = star::get_at<depth>(values);
    if (value_info.serial_name.view() == serial_name) {
      return value_info.value;
    }
    if constexpr (depth + 1 < value_num) {
      return rec(index_tag<depth + 1>);
    } else {
      return std::nullopt;
    }
  }(index_tag<0>);
}

/** The serial names of the enumerators of `Enum`, in declaration order. */
template<HasEnumInfo Enum>
inline constexpr auto serial_names = [] {
  using Info = EnumInfo<Enum>;
  constexpr std::size_t value_num = star::size<decltype(Info::values)>;
  return star::static_apply<value_num>([]<std::size_t... I> {
    return std::array<std::string_view, value_num>{
      star::get_at<I>(Info::values).serial_name.view()...};
  });
}();
} // namespace thes::reflect

namespace thes {
//==================================================================================================
// The customization points `argparse` looks up for the value type of an argument
//==================================================================================================

// Defining these here makes every reflected enumeration usable as the value type of a command-line
// argument without anyone having to ask for it: they are found by argument-dependent lookup, and
// this header is already in scope wherever such an enumeration is declared. Nothing about them
// reaches back into `argparse`, so this creates no dependency and `argparse` keeps needing nothing
// beyond the standard library.
//
// They take a `TypeTag` because that is what puts `thes` among the associated namespaces: lookup on
// the enumeration alone would only reach wherever its author declared it.

/** Converts `text` to an enumerator of `Enum` by its serial name. */
template<reflect::HasEnumInfo Enum>
[[nodiscard]] constexpr std::optional<Enum> parse_argument_value(TypeTag<Enum> /*tag*/,
                                                                 std::string_view text) {
  return reflect::enum_cast<Enum>(text);
}

/** The serial name of `value`, empty if it is no named enumerator of `Enum`. */
template<reflect::HasEnumInfo Enum>
[[nodiscard]] constexpr std::string_view argument_value_text(TypeTag<Enum> /*tag*/, Enum value) {
  return reflect::serial_name_of(value).value_or(std::string_view{});
}

/** The serial names of every enumerator, which the help text lists as the permitted values. */
template<reflect::HasEnumInfo Enum>
[[nodiscard]] constexpr std::span<const std::string_view>
argument_value_names(TypeTag<Enum> /*tag*/) {
  return reflect::serial_names<Enum>;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_REFLECTION_ENUM_HPP
