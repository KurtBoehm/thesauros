#ifndef INCLUDE_THESAUROS_MATH_FAST_HPP
#define INCLUDE_THESAUROS_MATH_FAST_HPP

#include <cmath>
#include <concepts>

#if __SSE__
#include <immintrin.h>
#endif

namespace thes::fast {
#ifdef __SSE__
namespace sse {
inline __m128 to_sse(float x) {
#if defined(__GNUC__) && !defined(__clang__)
  __m128 retval;
  asm("" : "=x"(retval) : "0"(x));
  return retval;
#elif defined(__clang__)
  float data[4];
  data[0] = x;
  return _mm_load_ps(data);
#else
  return _mm_set_ss(x);
#endif
}

inline __m128 fmadd_single(__m128 x, __m128 y, __m128 z) {
#ifdef __FMA__
  return _mm_fmadd_ss(x, y, z);
#else
  return to_sse(__builtin_fmaf(_mm_cvtss_f32(x), _mm_cvtss_f32(y), _mm_cvtss_f32(z)));
#endif
}

inline __m128 ternary_single(const __m128 mask, const __m128 a, const __m128 b) {
#if defined(__AVX512F__) && defined(__AVX512VL__)
  return _mm_castsi128_ps(
    _mm_ternarylogic_epi32(_mm_castps_si128(mask), _mm_castps_si128(a), _mm_castps_si128(b), 202));
#elif defined(__SSE4_1__)
  return _mm_blendv_ps(b, a, mask);
#else
  return _mm_or_ps(_mm_and_ps(mask, a), _mm_andnot_ps(mask, b));
#endif
}

inline float hypot_single(__m128 x, __m128 y) {
  const auto t1 = _mm_add_ss(y, y);
  const auto t2 = _mm_sub_ss(x, y);
  const auto arg0 = _mm_max_ss(t1, x);
  const auto mask = _mm_cmpge_ss(t1, x);
  const auto arg2sqrt = ternary_single(mask, t2, y);
  return _mm_cvtss_f32(_mm_sqrt_ss(fmadd_single(arg0, x, _mm_mul_ss(arg2sqrt, arg2sqrt))));
}

#ifdef __SSE2__
inline __m128d to_sse(double x) {
#if defined(__GNUC__) && !defined(__clang__)
  __m128d retval;
  asm("" : "=x"(retval) : "0"(x));
  return retval;
#elif defined(__clang__)
  double data[4];
  data[0] = x;
  return _mm_load_pd(data);
#else
  return _mm_set_sd(x);
#endif
}

inline __m128d fmadd_single(__m128d x, __m128d y, __m128d z) {
#ifdef __FMA__
  return _mm_fmadd_sd(x, y, z);
#else
  return to_sse(__builtin_fma(_mm_cvtsd_f64(x), _mm_cvtsd_f64(y), _mm_cvtsd_f64(z)));
#endif
}

inline __m128d ternary_single(const __m128d mask, const __m128d a, const __m128d b) {
#if defined(__AVX512F__) && defined(__AVX512VL__)
  return _mm_castsi128_pd(
    _mm_ternarylogic_epi64(_mm_castpd_si128(mask), _mm_castpd_si128(a), _mm_castpd_si128(b), 202));
#elif defined(__SSE4_1__)
  return _mm_blendv_pd(b, a, mask);
#else
  return _mm_or_pd(_mm_and_pd(mask, a), _mm_andnot_pd(mask, b));
#endif
}

inline double hypot_single(__m128d x, __m128d y) {
  const auto t1 = _mm_add_sd(y, y);
  const auto t2 = _mm_sub_sd(x, y);
  const auto arg0 = _mm_max_sd(t1, x);
  const auto mask = _mm_cmpge_sd(t1, x);
  const auto arg2sqrt = ternary_single(mask, t2, y);
  return _mm_cvtsd_f64(_mm_sqrt_pd(fmadd_single(arg0, x, _mm_mul_sd(arg2sqrt, arg2sqrt))));
}
#endif
} // namespace sse
#endif

template<std::floating_point T>
inline T fma(T x, T y, T z) {
  if constexpr (std::same_as<T, float>) {
    return __builtin_fmaf(x, y, z);
  }
  if constexpr (std::same_as<T, double>) {
    return __builtin_fma(x, y, z);
  }
  if constexpr (std::same_as<T, long double>) {
    return __builtin_fmal(x, y, z);
  }
}

template<typename T>
inline T sqrt(T value) {
#if __SSE__
  if constexpr (std::is_same_v<T, float>) {
    return _mm_cvtss_f32(_mm_sqrt_ss(sse::to_sse(value)));
  }
#endif
#if __SSE2__
  if constexpr (std::is_same_v<T, double>) {
    const auto vec = sse::to_sse(value);
    return _mm_cvtsd_f64(_mm_sqrt_sd(vec, vec));
  }
#endif
  return std::sqrt(value);
}

template<std::floating_point T>
inline T hypot(T x, T y) {
#ifdef __SSE__
  if constexpr (std::same_as<T, float>) {
    return sse::hypot_single(sse::to_sse(x), sse::to_sse(y));
  }
#endif

#ifdef __SSE2__
  if constexpr (std::same_as<T, double>) {
    return sse::hypot_single(sse::to_sse(x), sse::to_sse(y));
  }
#endif

  return std::hypot(x, y);
}

template<typename T>
inline T rsqrt(T value) {
  return T{1} / thes::fast::sqrt(value);
}

template<typename T>
inline T rsqrt(T /*value*/, T sqrt) {
  return T{1} / sqrt;
}
} // namespace thes::fast

#endif // INCLUDE_THESAUROS_MATH_FAST_HPP
