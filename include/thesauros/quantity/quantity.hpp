// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_QUANTITY_QUANTITY_HPP
#define INCLUDE_THESAUROS_QUANTITY_QUANTITY_HPP

#include <concepts>
#include <cstdint>
#include <numeric>
#include <ratio>
#include <type_traits>

namespace thes {
using std::atto;
using std::centi;
using std::deca;
using std::deci;
using std::exa;
using std::femto;
using std::giga;
using std::hecto;
using std::kilo;
using std::mega;
using std::micro;
using std::milli;
using std::nano;
using std::peta;
using std::pico;
using std::tera;

using kibi = std::ratio_multiply<std::ratio<1024>, std::ratio<1>>;
using mebi = std::ratio_multiply<std::ratio<1024>, kibi>;
using gibi = std::ratio_multiply<std::ratio<1024>, mebi>;
using tebi = std::ratio_multiply<std::ratio<1024>, gibi>;
using pebi = std::ratio_multiply<std::ratio<1024>, tebi>;
using exbi = std::ratio_multiply<std::ratio<1024>, pebi>;

namespace unit::base {
struct second;
struct metre;
struct kilogram;
struct ampere;
struct kelvin;
struct mole;
struct candela;
struct byte;
struct bit;
} // namespace unit::base

template<typename TMultiple, typename TBaseUnit>
struct Unit {
  using Multiple = TMultiple::type;
  using BaseUnit = TBaseUnit;

  using Type = Unit<Multiple, BaseUnit>;
};
template<typename TMultiple, typename TUnit>
using ScaledUnit =
  Unit<std::ratio_multiply<TMultiple, typename TUnit::Multiple>, typename TUnit::BaseUnit>;

namespace unit {
// SI multiples of second
using attosecond = Unit<atto, unit::base::second>;
using femtosecond = Unit<femto, unit::base::second>;
using picosecond = Unit<pico, unit::base::second>;
using nanosecond = Unit<nano, unit::base::second>;
using microsecond = Unit<micro, unit::base::second>;
using millisecond = Unit<milli, unit::base::second>;
using centisecond = Unit<centi, unit::base::second>;
using second = Unit<std::ratio<1>, unit::base::second>;
using decisecond = Unit<deci, unit::base::second>;
using decasecond = Unit<deca, unit::base::second>;
using hectosecond = Unit<hecto, unit::base::second>;
using kilosecond = Unit<kilo, unit::base::second>;
using megasecond = Unit<mega, unit::base::second>;
using gigasecond = Unit<giga, unit::base::second>;
using terasecond = Unit<tera, unit::base::second>;
using petasecond = Unit<peta, unit::base::second>;
using exasecond = Unit<exa, unit::base::second>;
// Non-SI time units accepted for use with SI
using minute = ScaledUnit<std::ratio<60>, second>;
using hour = ScaledUnit<std::ratio<60>, minute>;
using day = ScaledUnit<std::ratio<24>, hour>;

// SI multiples of metre
using attometre = Unit<atto, unit::base::metre>;
using femtometre = Unit<femto, unit::base::metre>;
using picometre = Unit<pico, unit::base::metre>;
using nanometre = Unit<nano, unit::base::metre>;
using micrometre = Unit<micro, unit::base::metre>;
using millimetre = Unit<milli, unit::base::metre>;
using centimetre = Unit<centi, unit::base::metre>;
using metre = Unit<std::ratio<1>, unit::base::metre>;
using decimetre = Unit<deci, unit::base::metre>;
using decametre = Unit<deca, unit::base::metre>;
using hectometre = Unit<hecto, unit::base::metre>;
using kilometre = Unit<kilo, unit::base::metre>;
using megametre = Unit<mega, unit::base::metre>;
using gigametre = Unit<giga, unit::base::metre>;
using terametre = Unit<tera, unit::base::metre>;
using petametre = Unit<peta, unit::base::metre>;
using exametre = Unit<exa, unit::base::metre>;

// SI multiples of byte
using byte = Unit<std::ratio<1>, unit::base::byte>;
using kilobyte = Unit<kilo, unit::base::byte>;
using megabyte = Unit<mega, unit::base::byte>;
using gigabyte = Unit<giga, unit::base::byte>;
using terabyte = Unit<tera, unit::base::byte>;
using petabyte = Unit<peta, unit::base::byte>;
using exabyte = Unit<exa, unit::base::byte>;
// Binary multiples of byte
using kibibyte = Unit<kibi, unit::base::byte>;
using mebibyte = Unit<mebi, unit::base::byte>;
using gibibyte = Unit<gibi, unit::base::byte>;
using tebibyte = Unit<tebi, unit::base::byte>;
using pebibyte = Unit<pebi, unit::base::byte>;
using exbibyte = Unit<exbi, unit::base::byte>;
} // namespace unit

template<typename TRep, typename TUnit>
struct Quantity {
  using Rep = TRep;
  using Unit = TUnit;

  explicit constexpr Quantity(TRep value) : value_(value) {}

  constexpr Rep count() const {
    return value_;
  }

private:
  Rep value_;
};

template<typename T>
struct IsQuantityTrait : std::false_type {};
template<typename TRep, typename TUnit>
struct IsQuantityTrait<Quantity<TRep, TUnit>> : std::true_type {};
template<typename T>
concept AnyQuantity = IsQuantityTrait<std::decay_t<T>>::value;

template<typename TU1, typename TU2>
struct UnitRatioTrait;
template<typename TMul1, typename TMul2, typename TBUnit>
struct UnitRatioTrait<Unit<TMul1, TBUnit>, Unit<TMul2, TBUnit>> {
  using Type = std::ratio_divide<TMul1, TMul2>;
};
template<typename TU1, typename TU2>
using UnitRatio = UnitRatioTrait<TU1, TU2>::Type;

template<typename TRep, typename TUnit>
struct IsBaseUnitQuantityTrait : std::false_type {};
template<typename TRep, typename TMul, typename TBUnit>
struct IsBaseUnitQuantityTrait<Quantity<TRep, Unit<TMul, TBUnit>>, TBUnit> : std::true_type {};

template<typename TOutQuant, typename TRep, typename TMul, typename TBUnit>
requires IsBaseUnitQuantityTrait<TOutQuant, TBUnit>::value
inline constexpr TOutQuant quantity_cast(const Quantity<TRep, Unit<TMul, TBUnit>>& sc) {
  using ToMultiple = TOutQuant::Unit::Multiple;
  using ToRep = TOutQuant::Rep;
  using CMul = std::ratio_divide<TMul, ToMultiple>;
  using CRep = std::common_type_t<ToRep, TRep, std::intmax_t>;

  if constexpr (CMul::num == 1) {
    if constexpr (CMul::den == 1) {
      return TOutQuant(static_cast<ToRep>(sc.count()));
    } else {
      return TOutQuant(
        static_cast<ToRep>(static_cast<CRep>(sc.count()) / static_cast<CRep>(CMul::den)));
    }
  } else {
    if constexpr (CMul::den == 1) {
      return TOutQuant(
        static_cast<ToRep>(static_cast<CRep>(sc.count()) * static_cast<CRep>(CMul::num)));
    } else {
      return TOutQuant(
        static_cast<ToRep>(static_cast<CRep>(sc.count()) * static_cast<CRep>(CMul::num) /
                           static_cast<CRep>(CMul::den)));
    }
  }
}
} // namespace thes

namespace std {
template<typename TRep1, typename TMul1, typename TRep2, typename TMul2, typename TBUnit>
requires(!std::same_as<thes::Unit<TRep1, TMul1>, thes::Unit<TRep2, TMul2>>)
struct common_type<thes::Quantity<TRep1, thes::Unit<TMul1, TBUnit>>,
                   thes::Quantity<TRep2, thes::Unit<TMul2, TBUnit>>> {
  static constexpr auto gcd_num = std::gcd(TMul1::num, TMul2::num);
  static constexpr auto gcd_den = std::gcd(TMul1::den, TMul2::den);
  using CRep = std::common_type_t<TRep1, TRep2>;
  using Ratio = std::ratio<gcd_num, (TMul1::den / gcd_den) * TMul2::den>::type;

  using type = thes::Quantity<CRep, thes::Unit<Ratio, TBUnit>>;
};

template<typename TRep, typename TUnit>
struct common_type<thes::Quantity<TRep, TUnit>, thes::Quantity<TRep, TUnit>> {
  using type = thes::Quantity<std::common_type_t<TRep>, typename TUnit::Type>;
};
} // namespace std

namespace thes {
template<typename TRep1, typename TMul1, typename TRep2, typename TMul2, typename TBUnit>
constexpr auto operator+(const Quantity<TRep1, Unit<TMul1, TBUnit>>& lhs,
                         const Quantity<TRep2, Unit<TMul2, TBUnit>>& rhs) {
  using Out =
    std::common_type_t<Quantity<TRep1, Unit<TMul1, TBUnit>>, Quantity<TRep2, Unit<TMul2, TBUnit>>>;
  return Out(quantity_cast<Out>(lhs).count() + quantity_cast<Out>(rhs).count());
}
template<typename TRep1, typename TMul1, typename TRep2, typename TMul2, typename TBUnit>
constexpr auto operator-(const Quantity<TRep1, Unit<TMul1, TBUnit>>& lhs,
                         const Quantity<TRep2, Unit<TMul2, TBUnit>>& rhs) {
  using Out =
    std::common_type_t<Quantity<TRep1, Unit<TMul1, TBUnit>>, Quantity<TRep2, Unit<TMul2, TBUnit>>>;
  return Out(quantity_cast<Out>(lhs).count() - quantity_cast<Out>(rhs).count());
}

template<typename TRep1, typename TMul, typename TRep2, typename TBUnit>
constexpr auto operator*(const Quantity<TRep1, Unit<TMul, TBUnit>>& q, const TRep2& s) {
  using Out = Quantity<std::common_type_t<TRep1, TRep2>, Unit<TMul, TBUnit>>;
  return quantity_cast<Out>(quantity_cast<Out>(q).count() * s);
}

template<typename TRep1, typename TMul, typename TRep2, typename TBUnit>
constexpr auto operator*(const TRep1& s, const Quantity<TRep2, Unit<TMul, TBUnit>>& q) {
  return q * s;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_QUANTITY_QUANTITY_HPP
