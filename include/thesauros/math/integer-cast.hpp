#ifndef INCLUDE_THESAUROS_MATH_INTEGER_CAST_HPP
#define INCLUDE_THESAUROS_MATH_INTEGER_CAST_HPP

#include <concepts>
#include <limits>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/utility/info-result.hpp"

namespace thes {
enum struct CastInfo { OKAY, TOO_SMALL, TOO_LARGE };
template<typename T>
using CastResult = InfoResult<T, CastInfo, CastInfo::OKAY>;

namespace detail {
template<std::integral TOut>
struct SafeCastWorker {
  using Ret = CastResult<TOut>;

  THES_ALWAYS_INLINE explicit constexpr SafeCastWorker(TOut v) : value_(v) {}

  THES_ALWAYS_INLINE constexpr void too_small() {
    info_ = CastInfo::TOO_SMALL;
  }
  THES_ALWAYS_INLINE constexpr void too_large() {
    info_ = CastInfo::TOO_LARGE;
  }
  THES_ALWAYS_INLINE constexpr Ret value() {
    return Ret{value_, info_};
  }

private:
  TOut value_;
  CastInfo info_{CastInfo::OKAY};
};
template<std::integral TOut>
struct SatCastWorker {
  using Ret = TOut;

  THES_ALWAYS_INLINE explicit constexpr SatCastWorker(TOut v) : value_(v) {}

  THES_ALWAYS_INLINE constexpr void too_small() {
    value_ = std::numeric_limits<TOut>::lowest();
  }
  THES_ALWAYS_INLINE constexpr void too_large() {
    value_ = std::numeric_limits<TOut>::max();
  }
  THES_ALWAYS_INLINE constexpr Ret value() {
    return value_;
  }

private:
  TOut value_;
};
} // namespace detail

template<typename TIn, typename TOut, typename TWorker>
struct CastTrait;
template<typename TIn, std::unsigned_integral TOut, typename TWorker>
struct CastTrait<TIn, TOut, TWorker> {
  using Ret = TWorker::Ret;

  static constexpr Ret cast(TIn in) {
    TWorker out{static_cast<TOut>(in)};
    if constexpr (std::signed_integral<TIn>) {
      if (in < 0) {
        out.too_small();
      }
    }
    if constexpr (std::numeric_limits<TOut>::digits < std::numeric_limits<TIn>::digits) {
      if (in > TIn{std::numeric_limits<TOut>::max()}) {
        out.too_large();
      }
    }
    return out.value();
  }
};
template<typename TIn, std::signed_integral TOut, typename TWorker>
struct CastTrait<TIn, TOut, TWorker> {
  using Ret = TWorker::Ret;

  static constexpr Ret cast(TIn in) {
    TWorker out{static_cast<TOut>(in)};
    if constexpr (std::numeric_limits<TOut>::digits < std::numeric_limits<TIn>::digits) {
      if constexpr (std::signed_integral<TIn>) {
        if (in < TIn{std::numeric_limits<TOut>::lowest()}) {
          out.too_small();
        }
      }
      if (in > TIn{std::numeric_limits<TOut>::max()}) {
        out.too_large();
      }
    }
    return out.value();
  }
};

template<std::integral TOut, std::integral TIn>
inline constexpr CastResult<TOut> safe_cast(TIn in) {
  return CastTrait<TIn, TOut, detail::SafeCastWorker<TOut>>::cast(in);
}
template<std::integral TOut, std::integral TIn>
inline constexpr TOut saturate_cast(TIn in) {
  return CastTrait<TIn, TOut, detail::SatCastWorker<TOut>>::cast(in);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_MATH_INTEGER_CAST_HPP
