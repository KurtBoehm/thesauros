// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_MACROPOLIS_SERIAL_VALUE_HPP
#define INCLUDE_THESAUROS_MACROPOLIS_SERIAL_VALUE_HPP

#include <stdexcept>
#include <type_traits>
#include <utility>

#include "thesauros/macropolis/enum.hpp"
#include "thesauros/macropolis/helpers.hpp"
#include "thesauros/macropolis/type.hpp"
#include "thesauros/static-ranges.hpp"
#include "thesauros/types/primitives.hpp"
#include "thesauros/types/type-tag.hpp"

namespace thes {
template<typename T>
struct SerialValueTrait {
  static constexpr T make(T&& value) {
    return value;
  }
  static constexpr const T& make(const T& value) {
    return value;
  }
  static constexpr T& make(T& value) {
    return value;
  }
};

template<typename T>
inline constexpr decltype(auto) serial_value(T&& value) {
  return SerialValueTrait<std::decay_t<T>>::make(std::forward<T>(value));
}

template<HasEnumInfo T>
struct SerialValueTrait<T> {
  static constexpr auto make(const T& value) {
    auto impl = [&](auto& rec, const auto& head, const auto&... tail) {
      if (value == head.value) {
        return head.serial_name.view();
      }
      if constexpr (sizeof...(tail) > 0) {
        return rec(rec, tail...);
      } else {
        throw std::invalid_argument("Unsupported value!");
      }
    };
    return EnumInfo<T>::values |
           star::apply([&](const auto&... members) { return impl(impl, members...); });
  }
};

template<HasTypeInfo T>
struct SerialValueTrait<TypeTag<T>> {
  static constexpr auto make(const T& /*value*/) {
    return TypeInfo<T>::serial_name.view();
  }
};

template<HasEnumInfo T>
struct SerialValueTrait<TypeTag<T>> {
  static constexpr auto make(const T& /*value*/) {
    return EnumInfo<T>::serial_name.view();
  }
};

#define TYPE_TAG_VALUE(TYPE) \
  template<> \
  struct SerialValueTrait<TypeTag<TYPE>> { \
    static constexpr auto name = THES_POLIS_STR(#TYPE); \
\
    static constexpr auto make(const TypeTag<TYPE>& /*tag*/) { \
      return name.view(); \
    } \
  }

TYPE_TAG_VALUE(u8);
TYPE_TAG_VALUE(u16);
TYPE_TAG_VALUE(u32);
TYPE_TAG_VALUE(u64);
TYPE_TAG_VALUE(u128);

TYPE_TAG_VALUE(i8);
TYPE_TAG_VALUE(i16);
TYPE_TAG_VALUE(i32);
TYPE_TAG_VALUE(i64);
TYPE_TAG_VALUE(i128);

TYPE_TAG_VALUE(f32);
TYPE_TAG_VALUE(f64);

#undef TYPE_TAG_VALUE
} // namespace thes

#endif // INCLUDE_THESAUROS_MACROPOLIS_SERIAL_VALUE_HPP
