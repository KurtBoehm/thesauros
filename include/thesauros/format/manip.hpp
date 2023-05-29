#ifndef INCLUDE_THESAUROS_FORMAT_MANIP_HPP
#define INCLUDE_THESAUROS_FORMAT_MANIP_HPP

#include <concepts>
#include <iomanip>
#include <ios>
#include <limits>
#include <ostream>

namespace thes::fmt {
struct PrecisionManip {
  int precision;

  void apply(std::ostream& stream) const {
    stream << std::setprecision(precision);
  }
};
struct FixedFloatManip {
  static void apply(std::ostream& stream) {
    stream << std::fixed;
  }
};
struct ScientificFloatManip {
  static void apply(std::ostream& stream) {
    stream << std::scientific;
  }
};
struct WidthManip {
  int width;

  void apply(std::ostream& stream) const {
    stream << std::setw(width);
  }
};
struct FillManip {
  char fill;

  void apply(std::ostream& stream) const {
    stream << std::setfill(fill);
  }
};
struct InternalAdjustmentManip {
  static void apply(std::ostream& stream) {
    stream << std::internal;
  }
};

template<typename T>
inline constexpr PrecisionManip full_precision{std::numeric_limits<T>::digits10};
inline constexpr FixedFloatManip fixed{};
inline constexpr ScientificFloatManip scientific{};
inline constexpr InternalAdjustmentManip internal{};

inline constexpr PrecisionManip precision(int precision) {
  return PrecisionManip{precision};
}
inline constexpr WidthManip width(int width) {
  return WidthManip{width};
}
inline constexpr FillManip fill(char fill) {
  return FillManip{fill};
}

template<typename TApp>
concept StreamApplier = requires(const TApp& app, std::ostream& s) {
  { app.apply(s) } -> std::same_as<void>;
};
} // namespace thes::fmt

#endif // INCLUDE_THESAUROS_FORMAT_MANIP_HPP
