// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_BENCHMARK_BENCHMARK_HPP
#define INCLUDE_THESAUROS_BENCHMARK_BENCHMARK_HPP

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cmath>
#include <concepts>
#include <type_traits>

#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/macropolis/platform.hpp"
#include "thesauros/math/integer-cast.hpp"
#include "thesauros/ranges/countdown.hpp"
#include "thesauros/types/primitives.hpp"

namespace thes::bench {
/**
 * Clobber memory, forcing the compiler to perform pending writes to global memory.
 *
 * Copies https://github.com/google/benchmark/blob/main/include/benchmark/utils.h#L36.
 */
[[THES_ALWAYS_INLINE]] inline void clobber_memory() {
  std::atomic_signal_fence(std::memory_order_acq_rel);
}

/**
 * Avoids optimizing the lvalue reference `value` away.
 *
 * Mirrors the respective overloads of `DoNotOptimize` in
 * https://github.com/google/benchmark/blob/main/include/benchmark/utils.h,
 * combining the GCC overloads via `constexpr if`.
 */
template<typename T>
[[THES_ALWAYS_INLINE]] inline void do_not_optimize(T& value) {
#if THES_CLANG
  asm volatile("" : "+r,m"(value) : : "memory");
#elif THES_GCC
  if constexpr (std::is_trivially_copyable_v<T> && sizeof(T) <= sizeof(T*)) {
    asm volatile("" : "+m,r"(value) : : "memory");
  } else {
    asm volatile("" : "+m"(value) : : "memory");
  }
#else
#error "Only Clang and GCC are supported!"
#endif
}

/**
 * Avoids optimizing the rvalue reference `value` away.
 *
 * Mirrors the respective overloads of `DoNotOptimize` in
 * https://github.com/google/benchmark/blob/main/include/benchmark/utils.h,
 * combining the GCC overloads via `constexpr if`.
 */
template<typename T>
[[THES_ALWAYS_INLINE]] inline void do_not_optimize(T&& value) {
#if THES_CLANG
  asm volatile("" : "+r,m"(value) : : "memory");
#elif THES_GCC
  if constexpr (std::is_trivially_copyable_v<T> && sizeof(T) <= sizeof(T*)) {
    asm volatile("" : "+m,r"(value) : : "memory");
  } else {
    asm volatile("" : "+m"(value) : : "memory");
  }
#else
#error "Only Clang and GCC are supported!"
#endif
}

/**
 * Avoids optimizing the const-lvalue reference `value` away.
 *
 * Mirrors the respective overloads of `DoNotOptimize` in
 * https://github.com/google/benchmark/blob/main/include/benchmark/utils.h,
 * combining the GCC overloads via `constexpr if`.
 */
template<typename T>
[[THES_ALWAYS_INLINE, deprecated("The const-lvalue-ref version of this function can permit "
                                 "undesired compiler optimizations")]] inline void
do_not_optimize(const T& value) {
#if THES_CLANG
  asm volatile("" : : "r,m"(value) : "memory");
#elif THES_GCC
  if constexpr (std::is_trivially_copyable_v<T> && sizeof(T) <= sizeof(T*)) {
    asm volatile("" : : "r,m"(value) : "memory");
  } else {
    asm volatile("" : : "m"(value) : "memory");
  }
#else
#error "Only Clang and GCC are supported!"
#endif
}

using Clock = std::chrono::steady_clock;
using Duration = Clock::duration;
using IterationCount = u64;
inline constexpr IterationCount max_iterations = IterationCount{1} << 40U;

struct Result {
  Duration total_duration;
  IterationCount iterations;
};

/**
 * Determines the number of iterations for the next run, given the last run’s duration and iteration
 * count and the minimum duration being targeted.
 *
 * Mirrors `PredictNumItersNeeded` in
 * https://github.com/google/benchmark/blob/main/src/benchmark_runner.cc,
 * but re-writes the logic in a way that avoids floating-point conversions where possible and is
 * generally simpler.
 */
inline IterationCount next_iters(Duration last_dur, IterationCount last_iters, Duration min_dur) {
  // If the last run took at most 10% of `min_dur`, the measurement is not precise enough yet;
  // increase the iteration count tenfold.
  if (10 * last_dur.count() <= min_dur.count()) {
    return std::min(10 * last_iters, max_iterations);
  }

  const auto to_seconds = [](const Duration& dur) {
    return std::chrono::duration<double>{dur}.count();
  };

  // When the last run took more than 10% of `min_dur`, aim for `1.4 * min_dur`.
  const double multiplier = 1.4 * to_seconds(min_dur) / to_seconds(last_dur);
  const IterationCount max_next =
    *thes::safe_cast<IterationCount>(std::llround(multiplier * static_cast<double>(last_iters)));

  // Clamp the iteration count so that it increases by at least one and is at most `max_iterations`.
  if (max_next >= max_iterations) {
    return max_iterations;
  }
  return std::max(max_next, last_iters + 1);
}

/**
 * Runs `f` repeatedly with an increasing number of iterations until the runtime across iterations
 * is at least `min_duration` or the number of iterations is at least `max_iterations`. Once one of
 * these conditions is met, the number of iterations and the total runtime across iterations are
 * returned. If `f` returns a value, `do_not_optimize` is called on that value automatically.
 *
 * Based on `DoOneRepetition` in
 * https://github.com/google/benchmark/blob/main/src/benchmark_runner.cc,
 * with significant simplifications: it only measures wall-clock time and only performs the
 * equivalent of one sample, which is Google Benchmark’s default anyway.
 */
template<typename F>
requires(std::invocable<F&>)
inline Result run(F&& f, Clock::duration min_duration = std::chrono::milliseconds{500}) {
  IterationCount iters = 1;

  while (true) {
    const auto t0 = Clock::now();
    for ([[maybe_unused]] const auto i : views::countdown(iters)) {
      if constexpr (std::is_void_v<std::invoke_result_t<F&>>) {
        f();
      } else {
        do_not_optimize(f());
      }
    }
    const auto last_dur = Clock::now() - t0;

    if (iters >= max_iterations || last_dur >= min_duration) {
      return {.total_duration = last_dur, .iterations = iters};
    }

    // The last run is not meaningful yet. Determine the number of iterations for the next run and
    // continue.
    const IterationCount last_iters = iters;
    iters = next_iters(last_dur, iters, min_duration);
    assert(iters > last_iters);
  }
}
} // namespace thes::bench

#endif // INCLUDE_THESAUROS_BENCHMARK_BENCHMARK_HPP
