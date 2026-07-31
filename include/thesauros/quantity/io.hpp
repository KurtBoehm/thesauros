// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_QUANTITY_IO_HPP
#define INCLUDE_THESAUROS_QUANTITY_IO_HPP

#include <cstdint>
#include <ratio>
#include <utility>

#include "thesauros/quantity/quantity.hpp"
#include "thesauros/string/static-string.hpp"

namespace thes {
template<typename T>
struct BaseUnitNameTrait;
template<typename T>
inline constexpr auto base_unit_name = BaseUnitNameTrait<T>::name;

#define BASE_UNIT_NAME(BASE_UNIT, NAME) \
  template<> \
  struct BaseUnitNameTrait<BASE_UNIT> { \
    static constexpr StaticString name{NAME}; \
  }

BASE_UNIT_NAME(unit::base::second, "s");
BASE_UNIT_NAME(unit::base::metre, "m");
BASE_UNIT_NAME(unit::base::kilogram, "kg");
BASE_UNIT_NAME(unit::base::ampere, "A");
BASE_UNIT_NAME(unit::base::kelvin, "K");
BASE_UNIT_NAME(unit::base::mole, "mol");
BASE_UNIT_NAME(unit::base::candela, "cd");
BASE_UNIT_NAME(unit::base::byte, "B");
BASE_UNIT_NAME(unit::base::bit, "bit");

#undef BASE_UNIT_NAME

template<typename T>
struct PrefixNameTrait;
template<typename T>
inline constexpr auto prefix_name = PrefixNameTrait<T>::name;

#define PREFIX_NAME(PREFIX, NAME) \
  template<> \
  struct PrefixNameTrait<PREFIX> { \
    static constexpr StaticString name{NAME}; \
  }

PREFIX_NAME(atto, "a");
PREFIX_NAME(femto, "f");
PREFIX_NAME(pico, "p");
PREFIX_NAME(nano, "n");
PREFIX_NAME(micro, "μ");
PREFIX_NAME(milli, "m");
PREFIX_NAME(centi, "c");
PREFIX_NAME(std::ratio<1>, "");
PREFIX_NAME(deci, "d");
PREFIX_NAME(deca, "da");
PREFIX_NAME(hecto, "h");
PREFIX_NAME(kilo, "k");
PREFIX_NAME(mega, "M");
PREFIX_NAME(giga, "G");
PREFIX_NAME(tera, "T");
PREFIX_NAME(peta, "P");
PREFIX_NAME(exa, "E");

PREFIX_NAME(kibi, "Ki");
PREFIX_NAME(mebi, "Mi");
PREFIX_NAME(gibi, "Gi");
PREFIX_NAME(tebi, "Ti");
PREFIX_NAME(pebi, "Pi");
PREFIX_NAME(exbi, "Ei");

#undef PREFIX_NAME

template<typename U>
struct UnitNameTrait;
template<typename T>
inline constexpr auto unit_name = UnitNameTrait<T>::name;

template<typename Mul, typename BUnit>
struct UnitNameTrait<Unit<Mul, BUnit>> {
  static constexpr auto name = prefix_name<Mul> + base_unit_name<BUnit>;
};

#define UNIT_NAME(UNIT, NAME) \
  template<> \
  struct UnitNameTrait<UNIT> { \
    static constexpr StaticString name{NAME}; \
  }

UNIT_NAME(unit::minute, "min");
UNIT_NAME(unit::hour, "h");
UNIT_NAME(unit::day, "d");

#undef UNIT_NAME

namespace detail {
template<std::intmax_t N>
constexpr std::intmax_t ratio_as_integer(std::ratio<N> /*r*/) {
  return N;
}
} // namespace detail

template<AnyQuantity Q>
struct SplitTimePrinter {
  using Unit = unit::millisecond;

  static constexpr std::intmax_t hour_limit =
    detail::ratio_as_integer(UnitRatio<unit::hour, Unit>{});
  static constexpr std::intmax_t min_limit =
    detail::ratio_as_integer(UnitRatio<unit::minute, Unit>{});
  static constexpr std::intmax_t sec_limit =
    detail::ratio_as_integer(UnitRatio<unit::second, Unit>{});

  explicit SplitTimePrinter(Q&& q) : quantity(std::forward<Q>(q)) {}

  Q quantity;
};

template<typename Q>
constexpr auto split_time(Q&& q) {
  return SplitTimePrinter<Q>(std::forward<Q>(q));
}
} // namespace thes

#endif // INCLUDE_THESAUROS_QUANTITY_IO_HPP
