#ifndef INCLUDE_THESAUROS_QUANTITY_QUANTITY_HPP
#define INCLUDE_THESAUROS_QUANTITY_QUANTITY_HPP

#include <concepts>
#include <numeric>
#include <ratio>

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

namespace base_unit {
struct second;
struct metre;
struct kilogram;
struct ampere;
struct kelvin;
struct mole;
struct candela;
struct byte;
struct bit;
} // namespace base_unit

template<typename TMultiple, typename TBaseUnit>
struct Unit {
  using Multiple = typename TMultiple::type;
  using BaseUnit = TBaseUnit;

  using Type = Unit<Multiple, BaseUnit>;
};
template<typename TMultiple, typename TUnit>
using ScaledUnit =
  Unit<std::ratio_multiply<TMultiple, typename TUnit::Multiple>, typename TUnit::BaseUnit>;

namespace unit {
// SI multiples of second
using attosecond = Unit<atto, base_unit::second>;
using femtosecond = Unit<femto, base_unit::second>;
using picosecond = Unit<pico, base_unit::second>;
using nanosecond = Unit<nano, base_unit::second>;
using microsecond = Unit<micro, base_unit::second>;
using millisecond = Unit<milli, base_unit::second>;
using centisecond = Unit<centi, base_unit::second>;
using second = Unit<std::ratio<1>, base_unit::second>;
using decisecond = Unit<deci, base_unit::second>;
using decasecond = Unit<deca, base_unit::second>;
using hectosecond = Unit<hecto, base_unit::second>;
using kilosecond = Unit<kilo, base_unit::second>;
using megasecond = Unit<mega, base_unit::second>;
using gigasecond = Unit<giga, base_unit::second>;
using terasecond = Unit<tera, base_unit::second>;
using petasecond = Unit<peta, base_unit::second>;
using exasecond = Unit<exa, base_unit::second>;
// Non-SI time units accepted for use with SI
using minute = ScaledUnit<std::ratio<60>, second>;
using hour = ScaledUnit<std::ratio<60>, minute>;
using day = ScaledUnit<std::ratio<24>, hour>;

// SI multiples of metre
using attometre = Unit<atto, base_unit::metre>;
using femtometre = Unit<femto, base_unit::metre>;
using picometre = Unit<pico, base_unit::metre>;
using nanometre = Unit<nano, base_unit::metre>;
using micrometre = Unit<micro, base_unit::metre>;
using millimetre = Unit<milli, base_unit::metre>;
using centimetre = Unit<centi, base_unit::metre>;
using metre = Unit<std::ratio<1>, base_unit::metre>;
using decimetre = Unit<deci, base_unit::metre>;
using decametre = Unit<deca, base_unit::metre>;
using hectometre = Unit<hecto, base_unit::metre>;
using kilometre = Unit<kilo, base_unit::metre>;
using megametre = Unit<mega, base_unit::metre>;
using gigametre = Unit<giga, base_unit::metre>;
using terametre = Unit<tera, base_unit::metre>;
using petametre = Unit<peta, base_unit::metre>;
using exametre = Unit<exa, base_unit::metre>;

// SI multiples of byte
using byte = Unit<std::ratio<1>, base_unit::byte>;
using kilobyte = Unit<kilo, base_unit::byte>;
using megabyte = Unit<mega, base_unit::byte>;
using gigabyte = Unit<giga, base_unit::byte>;
using terabyte = Unit<tera, base_unit::byte>;
using petabyte = Unit<peta, base_unit::byte>;
using exabyte = Unit<exa, base_unit::byte>;
// Binary multiples of byte
using kibibyte = Unit<kibi, base_unit::byte>;
using mebibyte = Unit<mebi, base_unit::byte>;
using gibibyte = Unit<gibi, base_unit::byte>;
using tebibyte = Unit<tebi, base_unit::byte>;
using pebibyte = Unit<pebi, base_unit::byte>;
using exbibyte = Unit<exbi, base_unit::byte>;
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
inline constexpr bool is_quantity = IsQuantityTrait<T>::value;

template<typename TU1, typename TU2>
struct UnitRatioTrait;
template<typename TMul1, typename TMul2, typename TBUnit>
struct UnitRatioTrait<Unit<TMul1, TBUnit>, Unit<TMul2, TBUnit>> {
  using Type = std::ratio_divide<TMul1, TMul2>;
};
template<typename TU1, typename TU2>
using UnitRatio = typename UnitRatioTrait<TU1, TU2>::Type;

template<typename TRep, typename TUnit>
struct IsBaseUnitQuantityTrait : std::false_type {};
template<typename TRep, typename TMul, typename TBUnit>
struct IsBaseUnitQuantityTrait<Quantity<TRep, Unit<TMul, TBUnit>>, TBUnit> : std::true_type {};

template<typename TOutQuant, typename TRep, typename TMul, typename TBUnit>
requires IsBaseUnitQuantityTrait<TOutQuant, TBUnit>::value
inline constexpr TOutQuant quantity_cast(const Quantity<TRep, Unit<TMul, TBUnit>>& sc) {
  using ToMultiple = typename TOutQuant::Unit::Multiple;
  using ToRep = typename TOutQuant::Rep;
  using CMul = std::ratio_divide<TMul, ToMultiple>;
  using CRep = std::common_type_t<ToRep, TRep, intmax_t>;

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
  using Ratio = typename std::ratio<gcd_num, (TMul1::den / gcd_den) * TMul2::den>::type;

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
