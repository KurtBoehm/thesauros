#ifndef INCLUDE_THESAUROS_MATH_FINITE_HPP
#define INCLUDE_THESAUROS_MATH_FINITE_HPP

#include <limits>

#include "thesauros/macropolis/platform.hpp"
#include "thesauros/math/fast.hpp"

namespace thes {
template<typename T>
requires std::numeric_limits<T>::is_iec559
inline bool is_finite(const T x) {
  // Basic idea: inf - inf = (-inf) - (-inf) = nan - nan = nan,
  // whereas x - x = ±0 (depending on rounding)
  return x - x == 0; // NOLINT(misc-redundant-expression)
}

template<typename T>
requires std::numeric_limits<T>::is_iec559
inline T make_finite(const T x) {
#if THES_X86_SSE
  if constexpr (std::is_same_v<T, float>) {
    const auto v = fast::sse::to_sse(x);
    const auto b = fast::sse::cmp_equal(_mm_sub_ss(v, v), _mm_setzero_ps());
    return _mm_cvtss_f32(fast::sse::zero_mask(b, v));
  }
#endif

#if THES_X86_SSE2
  if constexpr (std::is_same_v<T, double>) {
    const auto v = fast::sse::to_sse(x);
    const auto b = fast::sse::cmp_equal(_mm_sub_sd(v, v), _mm_setzero_pd());
    return _mm_cvtsd_f64(fast::sse::zero_mask(b, v));
  }
#endif

  return is_finite(x) ? x : T{};
}
} // namespace thes

#endif // INCLUDE_THESAUROS_MATH_FINITE_HPP
