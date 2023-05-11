#ifndef INCLUDE_THESAUROS_UTILITY_SAFE_CAST_HPP
#define INCLUDE_THESAUROS_UTILITY_SAFE_CAST_HPP

#include <limits>
#include <ostream>

#include "thesauros/utility/info-result.hpp"
#include "tl/expected.hpp"

namespace thes {
enum class CastInfo { OKAY, TOO_SMALL, TOO_LARGE };
inline std::ostream& operator<<(std::ostream& s, CastInfo err) {
  using Under = std::underlying_type_t<CastInfo>;

  switch (err) {
  case CastInfo::OKAY: return s << "OKAY";
  case CastInfo::TOO_SMALL: return s << "TOO_SMALL";
  case CastInfo::TOO_LARGE: return s << "TOO_LARGE";
  default: return s << "CastErrorCode(" << static_cast<Under>(err) << ")";
  }
}

template<typename TIn, typename TOut>
struct SafeCast;
template<typename TIn, typename TOut>
requires std::unsigned_integral<TOut>
struct SafeCast<TIn, TOut> {
  using Ret = InfoResult<TOut, CastInfo>;

  static constexpr Ret cast(TIn in) {
    CastInfo info = CastInfo::OKAY;
    if constexpr (std::signed_integral<TIn>) {
      if (in < 0) {
        info = CastInfo::TOO_SMALL;
      }
    }
    if constexpr (std::numeric_limits<TOut>::digits < std::numeric_limits<TIn>::digits) {
      if (in > TIn{std::numeric_limits<TOut>::max()}) {
        info = CastInfo::TOO_LARGE;
      }
    }
    return {static_cast<TOut>(in), info};
  }
};
template<typename TIn, typename TOut>
requires std::signed_integral<TOut>
struct SafeCast<TIn, TOut> {
  using Ret = InfoResult<TOut, CastInfo>;

  static constexpr Ret cast(TIn in) {
    CastInfo info = CastInfo::OKAY;
    if constexpr (std::numeric_limits<TOut>::digits < std::numeric_limits<TIn>::digits) {
      if constexpr (std::signed_integral<TIn>) {
        if (in < TIn{std::numeric_limits<TOut>::min()}) {
          info = CastInfo::TOO_SMALL;
        }
      }
      if (in > TIn{std::numeric_limits<TOut>::max()}) {
        info = CastInfo::TOO_LARGE;
      }
    }
    return {static_cast<TOut>(in), info};
  }
};

template<typename TOut, typename TIn>
inline constexpr InfoResult<TOut, CastInfo> safe_cast(TIn in) {
  return SafeCast<TIn, TOut>::cast(in);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_SAFE_CAST_HPP
