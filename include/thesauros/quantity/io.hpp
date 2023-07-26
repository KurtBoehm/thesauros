#ifndef INCLUDE_THESAUROS_QUANTITY_IO_HPP
#define INCLUDE_THESAUROS_QUANTITY_IO_HPP

#include <cstdint>
#include <iomanip>
#include <ostream>
#include <ratio>
#include <type_traits>
#include <utility>

#include "quantity.hpp"

namespace thes {
template<typename T>
struct BaseUnitPrint;

#define BASE_UNIT_PRINT(BASE_UNIT, NAME) \
  template<> \
  struct BaseUnitPrint<BASE_UNIT> { \
    static std::ostream& put(std::ostream& s) { \
      return s << (NAME); \
    } \
  }

BASE_UNIT_PRINT(base_unit::second, "s");
BASE_UNIT_PRINT(base_unit::metre, "m");
BASE_UNIT_PRINT(base_unit::kilogram, "kg");
BASE_UNIT_PRINT(base_unit::ampere, "A");
BASE_UNIT_PRINT(base_unit::kelvin, "K");
BASE_UNIT_PRINT(base_unit::mole, "mol");
BASE_UNIT_PRINT(base_unit::candela, "cd");
BASE_UNIT_PRINT(base_unit::byte, "B");
BASE_UNIT_PRINT(base_unit::bit, "bit");

#undef BASE_UNIT_PRINT

template<typename T>
struct PrefixPrint;

#define PREFIX_PRINT(PREFIX, NAME) \
  template<> \
  struct PrefixPrint<PREFIX> { \
    static std::ostream& put(std::ostream& s) { \
      return s << (NAME); \
    } \
  }

PREFIX_PRINT(atto, "a");
PREFIX_PRINT(femto, "f");
PREFIX_PRINT(pico, "p");
PREFIX_PRINT(nano, "n");
PREFIX_PRINT(micro, "μ");
PREFIX_PRINT(milli, "m");
PREFIX_PRINT(centi, "c");
PREFIX_PRINT(std::ratio<1>, "");
PREFIX_PRINT(deci, "d");
PREFIX_PRINT(deca, "da");
PREFIX_PRINT(hecto, "h");
PREFIX_PRINT(kilo, "k");
PREFIX_PRINT(mega, "M");
PREFIX_PRINT(giga, "G");
PREFIX_PRINT(tera, "T");
PREFIX_PRINT(peta, "P");
PREFIX_PRINT(exa, "E");

PREFIX_PRINT(kibi, "Ki");
PREFIX_PRINT(mebi, "Mi");
PREFIX_PRINT(gibi, "Gi");
PREFIX_PRINT(tebi, "Ti");
PREFIX_PRINT(pebi, "Pi");
PREFIX_PRINT(exbi, "Ei");

#undef SI_PREFIX_PRINT

template<typename TUnit>
struct UnitPrint;

template<typename TMul, typename TBUnit>
struct UnitPrint<Unit<TMul, TBUnit>> {
  static std::ostream& put(std::ostream& s) {
    PrefixPrint<TMul>::put(s);
    BaseUnitPrint<TBUnit>::put(s);
    return s;
  }
};

#define UNIT_PRINT(UNIT, NAME) \
  template<> \
  struct UnitPrint<UNIT> { \
    static std::ostream& put(std::ostream& s) { \
      return s << (NAME); \
    } \
  }

UNIT_PRINT(unit::minute, "min");
UNIT_PRINT(unit::hour, "h");
UNIT_PRINT(unit::day, "d");

#undef UNIT_PRINT

template<typename TRep, typename TUnit>
inline std::ostream& operator<<(std::ostream& s, const Quantity<TRep, TUnit>& q) {
  s << q.count() << " ";
  UnitPrint<TUnit>::put(s);
  return s;
}

template<typename TQuantity>
requires is_quantity<std::decay_t<TQuantity>>
struct SplitTimePrinter {
  using Unit = unit::millisecond;

private:
  template<std::intmax_t tNum>
  static constexpr std::intmax_t as_integer(std::ratio<tNum> /*r*/) {
    return tNum;
  }

public:
  static constexpr auto hour_limit = as_integer(UnitRatio<unit::hour, Unit>{});
  static constexpr auto min_limit = as_integer(UnitRatio<unit::minute, Unit>{});
  static constexpr auto sec_limit = as_integer(UnitRatio<unit::second, Unit>{});

  explicit SplitTimePrinter(TQuantity&& q) : quantity(std::forward<TQuantity>(q)) {}

  friend std::ostream& operator<<(std::ostream& s, const SplitTimePrinter& smp) {
    std::intmax_t milliq = quantity_cast<Quantity<std::intmax_t, Unit>>(smp.quantity).count();

    if (milliq < 0) {
      s << "-";
      milliq = -milliq;
    }

    const auto hour_div = milliq / hour_limit;
    const auto hour_mod = milliq % hour_limit;
    s << hour_div << ":";

    const auto min_div = hour_mod / min_limit;
    const auto min_mod = hour_mod % min_limit;
    s << std::setw(2) << std::setfill('0') << min_div << ":";

    const auto sec_div = min_mod / sec_limit;
    const auto sec_mod = min_mod % sec_limit;
    s << std::setw(2) << std::setfill('0') << sec_div;

    if (milliq < min_limit) {
      s << "." << std::setw(3) << std::setfill('0') << sec_mod;
    }

    return s;
  }

  TQuantity quantity;
};

template<typename TQuantity>
inline constexpr auto split_time(TQuantity&& q) {
  return SplitTimePrinter<TQuantity>(std::forward<TQuantity>(q));
}
} // namespace thes

#endif // INCLUDE_THESAUROS_QUANTITY_IO_HPP
