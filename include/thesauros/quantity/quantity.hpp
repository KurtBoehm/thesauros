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

using kibi = std::ratio_multiply<std::ratio<1024>, std::ratio<1>>; // NOLINT(*-identifier-naming)
using mebi = std::ratio_multiply<std::ratio<1024>, kibi>; // NOLINT(*-identifier-naming)
using gibi = std::ratio_multiply<std::ratio<1024>, mebi>; // NOLINT(*-identifier-naming)
using tebi = std::ratio_multiply<std::ratio<1024>, gibi>; // NOLINT(*-identifier-naming)
using pebi = std::ratio_multiply<std::ratio<1024>, tebi>; // NOLINT(*-identifier-naming)
using exbi = std::ratio_multiply<std::ratio<1024>, pebi>; // NOLINT(*-identifier-naming)

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

template<typename Mul, typename BaseU>
struct Unit {
  using Multiple = Mul::type;
  using BaseUnit = BaseU;

  using Type = Unit<Multiple, BaseUnit>;
};
template<typename Mul, typename U>
using ScaledUnit = Unit<std::ratio_multiply<Mul, typename U::Multiple>, typename U::BaseUnit>;

namespace unit {
// SI multiples of second
using attosecond = Unit<atto, unit::base::second>; // NOLINT(*-identifier-naming)
using femtosecond = Unit<femto, unit::base::second>; // NOLINT(*-identifier-naming)
using picosecond = Unit<pico, unit::base::second>; // NOLINT(*-identifier-naming)
using nanosecond = Unit<nano, unit::base::second>; // NOLINT(*-identifier-naming)
using microsecond = Unit<micro, unit::base::second>; // NOLINT(*-identifier-naming)
using millisecond = Unit<milli, unit::base::second>; // NOLINT(*-identifier-naming)
using centisecond = Unit<centi, unit::base::second>; // NOLINT(*-identifier-naming)
using second = Unit<std::ratio<1>, unit::base::second>; // NOLINT(*-identifier-naming)
using decisecond = Unit<deci, unit::base::second>; // NOLINT(*-identifier-naming)
using decasecond = Unit<deca, unit::base::second>; // NOLINT(*-identifier-naming)
using hectosecond = Unit<hecto, unit::base::second>; // NOLINT(*-identifier-naming)
using kilosecond = Unit<kilo, unit::base::second>; // NOLINT(*-identifier-naming)
using megasecond = Unit<mega, unit::base::second>; // NOLINT(*-identifier-naming)
using gigasecond = Unit<giga, unit::base::second>; // NOLINT(*-identifier-naming)
using terasecond = Unit<tera, unit::base::second>; // NOLINT(*-identifier-naming)
using petasecond = Unit<peta, unit::base::second>; // NOLINT(*-identifier-naming)
using exasecond = Unit<exa, unit::base::second>; // NOLINT(*-identifier-naming)
// Non-SI time units accepted for use with SI
using minute = ScaledUnit<std::ratio<60>, second>; // NOLINT(*-identifier-naming)
using hour = ScaledUnit<std::ratio<60>, minute>; // NOLINT(*-identifier-naming)
using day = ScaledUnit<std::ratio<24>, hour>; // NOLINT(*-identifier-naming)

// SI multiples of metre
using attometre = Unit<atto, unit::base::metre>; // NOLINT(*-identifier-naming)
using femtometre = Unit<femto, unit::base::metre>; // NOLINT(*-identifier-naming)
using picometre = Unit<pico, unit::base::metre>; // NOLINT(*-identifier-naming)
using nanometre = Unit<nano, unit::base::metre>; // NOLINT(*-identifier-naming)
using micrometre = Unit<micro, unit::base::metre>; // NOLINT(*-identifier-naming)
using millimetre = Unit<milli, unit::base::metre>; // NOLINT(*-identifier-naming)
using centimetre = Unit<centi, unit::base::metre>; // NOLINT(*-identifier-naming)
using metre = Unit<std::ratio<1>, unit::base::metre>; // NOLINT(*-identifier-naming)
using decimetre = Unit<deci, unit::base::metre>; // NOLINT(*-identifier-naming)
using decametre = Unit<deca, unit::base::metre>; // NOLINT(*-identifier-naming)
using hectometre = Unit<hecto, unit::base::metre>; // NOLINT(*-identifier-naming)
using kilometre = Unit<kilo, unit::base::metre>; // NOLINT(*-identifier-naming)
using megametre = Unit<mega, unit::base::metre>; // NOLINT(*-identifier-naming)
using gigametre = Unit<giga, unit::base::metre>; // NOLINT(*-identifier-naming)
using terametre = Unit<tera, unit::base::metre>; // NOLINT(*-identifier-naming)
using petametre = Unit<peta, unit::base::metre>; // NOLINT(*-identifier-naming)
using exametre = Unit<exa, unit::base::metre>; // NOLINT(*-identifier-naming)

// SI multiples of byte
using byte = Unit<std::ratio<1>, unit::base::byte>; // NOLINT(*-identifier-naming)
using kilobyte = Unit<kilo, unit::base::byte>; // NOLINT(*-identifier-naming)
using megabyte = Unit<mega, unit::base::byte>; // NOLINT(*-identifier-naming)
using gigabyte = Unit<giga, unit::base::byte>; // NOLINT(*-identifier-naming)
using terabyte = Unit<tera, unit::base::byte>; // NOLINT(*-identifier-naming)
using petabyte = Unit<peta, unit::base::byte>; // NOLINT(*-identifier-naming)
using exabyte = Unit<exa, unit::base::byte>; // NOLINT(*-identifier-naming)
// Binary multiples of byte
using kibibyte = Unit<kibi, unit::base::byte>; // NOLINT(*-identifier-naming)
using mebibyte = Unit<mebi, unit::base::byte>; // NOLINT(*-identifier-naming)
using gibibyte = Unit<gibi, unit::base::byte>; // NOLINT(*-identifier-naming)
using tebibyte = Unit<tebi, unit::base::byte>; // NOLINT(*-identifier-naming)
using pebibyte = Unit<pebi, unit::base::byte>; // NOLINT(*-identifier-naming)
using exbibyte = Unit<exbi, unit::base::byte>; // NOLINT(*-identifier-naming)
} // namespace unit

template<typename R, typename U>
struct Quantity {
  using Rep = R;
  using Unit = U;

  explicit constexpr Quantity(R value) : value_(value) {}

  constexpr Rep count() const {
    return value_;
  }

private:
  Rep value_;
};

template<typename T>
struct IsQuantityTrait : std::false_type {};
template<typename R, typename U>
struct IsQuantityTrait<Quantity<R, U>> : std::true_type {};
template<typename T>
concept AnyQuantity = IsQuantityTrait<std::decay_t<T>>::value;

template<typename U1, typename U2>
struct UnitRatioTrait;
template<typename Mul1, typename Mul2, typename BUnit>
struct UnitRatioTrait<Unit<Mul1, BUnit>, Unit<Mul2, BUnit>> {
  using Type = std::ratio_divide<Mul1, Mul2>;
};
template<typename U1, typename U2>
using UnitRatio = UnitRatioTrait<U1, U2>::Type;

template<typename R, typename U>
struct IsBaseUnitQuantityTrait : std::false_type {};
template<typename R, typename Mul, typename BUnit>
struct IsBaseUnitQuantityTrait<Quantity<R, Unit<Mul, BUnit>>, BUnit> : std::true_type {};

template<typename OutQuant, typename R, typename Mul, typename BUnit>
requires IsBaseUnitQuantityTrait<OutQuant, BUnit>::value
constexpr OutQuant quantity_cast(const Quantity<R, Unit<Mul, BUnit>>& sc) {
  using ToMultiple = OutQuant::Unit::Multiple;
  using ToRep = OutQuant::Rep;
  using CMul = std::ratio_divide<Mul, ToMultiple>;
  using CRep = std::common_type_t<ToRep, R, std::intmax_t>;

  if constexpr (CMul::num == 1) {
    if constexpr (CMul::den == 1) {
      return OutQuant(static_cast<ToRep>(sc.count()));
    } else {
      return OutQuant(
        static_cast<ToRep>(static_cast<CRep>(sc.count()) / static_cast<CRep>(CMul::den)));
    }
  } else {
    if constexpr (CMul::den == 1) {
      return OutQuant(
        static_cast<ToRep>(static_cast<CRep>(sc.count()) * static_cast<CRep>(CMul::num)));
    } else {
      return OutQuant(
        static_cast<ToRep>(static_cast<CRep>(sc.count()) * static_cast<CRep>(CMul::num) /
                           static_cast<CRep>(CMul::den)));
    }
  }
}
} // namespace thes

namespace std {
template<typename Rep1, typename Mul1, typename Rep2, typename Mul2, typename BUnit>
requires(!std::same_as<thes::Unit<Rep1, Mul1>, thes::Unit<Rep2, Mul2>>)
struct common_type<thes::Quantity<Rep1, thes::Unit<Mul1, BUnit>>,
                   thes::Quantity<Rep2, thes::Unit<Mul2, BUnit>>> {
  static constexpr auto gcd_num = std::gcd(Mul1::num, Mul2::num);
  static constexpr auto gcd_den = std::gcd(Mul1::den, Mul2::den);
  using CRep = std::common_type_t<Rep1, Rep2>;
  using Ratio = std::ratio<gcd_num, (Mul1::den / gcd_den) * Mul2::den>::type;

  using type = thes::Quantity<CRep, thes::Unit<Ratio, BUnit>>;
};

template<typename R, typename U>
struct common_type<thes::Quantity<R, U>, thes::Quantity<R, U>> {
  using type = thes::Quantity<std::common_type_t<R>, typename U::Type>;
};
} // namespace std

namespace thes {
template<typename Rep1, typename Mul1, typename Rep2, typename Mul2, typename BUnit>
constexpr auto operator+(const Quantity<Rep1, Unit<Mul1, BUnit>>& lhs,
                         const Quantity<Rep2, Unit<Mul2, BUnit>>& rhs) {
  using Out =
    std::common_type_t<Quantity<Rep1, Unit<Mul1, BUnit>>, Quantity<Rep2, Unit<Mul2, BUnit>>>;
  return Out(quantity_cast<Out>(lhs).count() + quantity_cast<Out>(rhs).count());
}
template<typename Rep1, typename Mul1, typename Rep2, typename Mul2, typename BUnit>
constexpr auto operator-(const Quantity<Rep1, Unit<Mul1, BUnit>>& lhs,
                         const Quantity<Rep2, Unit<Mul2, BUnit>>& rhs) {
  using Out =
    std::common_type_t<Quantity<Rep1, Unit<Mul1, BUnit>>, Quantity<Rep2, Unit<Mul2, BUnit>>>;
  return Out(quantity_cast<Out>(lhs).count() - quantity_cast<Out>(rhs).count());
}

template<typename Rep1, typename Mul, typename Rep2, typename BUnit>
constexpr auto operator*(const Quantity<Rep1, Unit<Mul, BUnit>>& q, const Rep2& s) {
  using Out = Quantity<std::common_type_t<Rep1, Rep2>, Unit<Mul, BUnit>>;
  return quantity_cast<Out>(quantity_cast<Out>(q).count() * s);
}

template<typename Rep1, typename Mul, typename Rep2, typename BUnit>
constexpr auto operator*(const Rep1& s, const Quantity<Rep2, Unit<Mul, BUnit>>& q) {
  return q * s;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_QUANTITY_QUANTITY_HPP
