#ifndef INCLUDE_THESAUROS_UTILITY_SAFE_CAST_HPP
#define INCLUDE_THESAUROS_UTILITY_SAFE_CAST_HPP

#include <limits>
#include <ostream>

#include "tl/expected.hpp"

namespace thes {
enum class CastErrorCode { TOO_SMALL, TOO_LARGE };
inline std::ostream& operator<<(std::ostream& s, CastErrorCode err) {
  using Under = std::underlying_type_t<CastErrorCode>;

  switch (err) {
  case CastErrorCode::TOO_SMALL: return s << "TOO_SMALL";
  case CastErrorCode::TOO_LARGE: return s << "TOO_LARGE";
  default: return s << "CastErrorCode(" << static_cast<Under>(err) << ")";
  }
}

template<typename TIn, typename TOut>
struct SafeCast;
template<typename TIn, typename TOut>
requires std::unsigned_integral<TOut>
struct SafeCast<TIn, TOut> {
  using Ret = tl::expected<TOut, CastErrorCode>;

  static constexpr Ret cast(TIn in) {
    Ret ret = static_cast<TOut>(in);
    if constexpr (std::signed_integral<TIn>) {
      if (in < 0) {
        ret = tl::unexpected(CastErrorCode::TOO_SMALL);
      }
    }
    if constexpr (std::numeric_limits<TOut>::digits < std::numeric_limits<TIn>::digits) {
      if (in > TIn{std::numeric_limits<TOut>::max()}) {
        ret = tl::unexpected(CastErrorCode::TOO_LARGE);
      }
    }
    return ret;
  }
};
template<typename TIn, typename TOut>
requires std::signed_integral<TOut>
struct SafeCast<TIn, TOut> {
  using Ret = tl::expected<TOut, CastErrorCode>;

  static constexpr Ret cast(TIn in) {
    Ret ret = static_cast<TOut>(in);
    if constexpr (std::numeric_limits<TOut>::digits < std::numeric_limits<TIn>::digits) {
      if constexpr (std::signed_integral<TIn>) {
        if (in < TIn{std::numeric_limits<TOut>::min()}) {
          ret = tl::unexpected(CastErrorCode::TOO_SMALL);
        }
      }
      if (in > TIn{std::numeric_limits<TOut>::max()}) {
        ret = tl::unexpected(CastErrorCode::TOO_LARGE);
      }
    }
    return ret;
  }
};

template<typename TOut, typename TIn>
inline constexpr tl::expected<TOut, CastErrorCode> safe_cast(TIn in) {
  return SafeCast<TIn, TOut>::cast(in);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_SAFE_CAST_HPP
