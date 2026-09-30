// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_EXECUTION_EXECUTOR_FIXED_THREAD_POOL_HPP
#define INCLUDE_THESAUROS_EXECUTION_EXECUTOR_FIXED_THREAD_POOL_HPP

#include <atomic>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>

#include "thesauros/charconv/concat.hpp"
#include "thesauros/concepts/nothrow.hpp"
#include "thesauros/containers/array/fixed-alloc.hpp"
#include "thesauros/execution/system/affinity.hpp"
#include "thesauros/execution/system/spin.hpp"
#include "thesauros/execution/system/this-thread.hpp"
#include "thesauros/math/integer-cast.hpp"
#include "thesauros/ranges/countdown.hpp"
#include "thesauros/ranges/index-type.hpp"
#include "thesauros/ranges/indices.hpp"
#include "thesauros/types/empty.hpp"
#include "thesauros/types/primitives.hpp"

namespace thes {
/**
 * A thread pool of a fixed size that aims to make the dispatch of parallel regions as cheap as
 * possible to maximize the performance of short parallel sections. Its goal is to match OpenMP’s
 * performance on platforms on which it performs well (Linux) while providing performance
 * portability for platforms with limited support or unsatisfactory performance (macOS).
 *
 * To this end, this class has the following properties:
 * - It uses the calling thread as thread `0` when executing to avoid over-subscribing the system.
 * - The task is passed as a function pointer together with a pointer to the callable, which lives
 *   on the caller’s stack for the duration of the blocking execution to avoid dynamic allocations.
 * - Threads spin for `spin_count` iterations before parking on `std::atomic::wait`; back-to-back
 *   parallel regions therefore never enter the kernel while a pool left idle stops consuming CPU
 *   time.
 *
 * `execute` must only be called from the thread that created the pool; calling it from within a
 * task is always forbidden. The task needs to be callable in a non-throwing fashion; the thread
 * pool does not implement exception handling.
 */
struct FixedThreadPool {
  /**
   * The number of iterations spent spinning before parking, which trades the latency of the next
   * parallel region against the CPU time burnt while waiting for it, which is mitigated by
   * `spin_pause()`. The default should correspond to well below one millisecond, although the exact
   * duration depends on many factors.
   */
  static constexpr std::size_t default_spin_count = 1UZ << 14UZ;
  /**
   * The number of bits allocated to the number of participating threads in the dispatch state,
   * trading the maximum number of threads against the number of epochs before wrap-around.
   * 2¹⁶ - 1 = 65535 threads is sufficient even for the largest shared-memory systems while 2⁴⁸ is
   * enough as the epoch counter even with very short parallel sections: at 1 µs per parallel
   * section, this would only be exhausted after about 8.9 years.
   */
  static constexpr std::size_t used_thread_bits = 16UZ;
  /** The largest supported thread count, constrained by the packing of the dispatch state. */
  static constexpr std::size_t max_thread_num = (1UZ << used_thread_bits) - 1;

  using Threads = FixedAllocArray<std::jthread>;

  /**
   * Create a pool of `size` threads, one of which is the calling thread.
   * @param cpu_sets The CPU sets to pin the threads to, with the first entry applying to the
   *                 calling thread. `Empty` keeps the affinities unchanged.
   */
  template<typename CpuSets = Empty>
  explicit FixedThreadPool(std::size_t size, const CpuSets& cpu_sets = {},
                           std::size_t spin_count = default_spin_count)
      : thread_num_{size}, spin_count_{spin_count},
        workers_{Threads::create_with_capacity(worker_num(size))} {
    if constexpr (!std::same_as<CpuSets, Empty>) {
      if (size > cpu_sets.size()) {
        throw std::invalid_argument{cat(size, " threads have been requested, but there are only ",
                                        cpu_sets.size(), " entries in the CPU set!")};
      }
    }

    const std::size_t workers = worker_num(size);
    for (const std::size_t i : views::indices(workers)) {
      workers_.emplace_back([this, index = i + 1] { work(index); });
    }

    if constexpr (!std::same_as<CpuSets, Empty>) {
      using Index = ranges::RangeIndex<CpuSets>;
      if (size > 0) {
        (void)set_affinity(this_thread_native_handle(), cpu_sets[Index{0}]);
      }
      for (const std::size_t i : views::indices(workers)) {
        (void)set_affinity(workers_[i], cpu_sets[*safe_cast<Index>(i + 1)]);
      }
    }
  }

  /** Create a pool pinning each thread to one of the CPUs described by `cpu_infos`. */
  template<typename CpuInfos = Empty>
  static FixedThreadPool from_cpu_infos(std::size_t size, CpuInfos&& cpu_infos = {},
                                        std::size_t spin_count = default_spin_count) {
    if constexpr (std::same_as<std::remove_cvref_t<CpuInfos>, Empty>) {
      return FixedThreadPool{size, Empty{}, spin_count};
    } else {
      return FixedThreadPool{
        size,
        std::views::transform(std::forward<CpuInfos>(cpu_infos),
                              [](auto cpu) { return CpuSet::single_set(cpu.id); }),
        spin_count,
      };
    }
  }

  FixedThreadPool(const FixedThreadPool&) = delete;
  FixedThreadPool(FixedThreadPool&&) = delete;
  FixedThreadPool& operator=(const FixedThreadPool&) = delete;
  FixedThreadPool& operator=(FixedThreadPool&&) = delete;

  ~FixedThreadPool() {
    stop_.store(true, std::memory_order_relaxed);
    state_.fetch_add(epoch_step, std::memory_order_release);
    state_.notify_all();
  }

  [[nodiscard]] std::size_t thread_num() const noexcept {
    return thread_num_;
  }

  /**
   * Run `task` on `used_thread_num` threads (which may be at most the thread count the pool has
   * been created with, which is also used as the fallback), blocking until all participating
   * threads are done. `task` is called with the index of the thread it is executed on, making this
   * an appropriate index for accesses into a shared array, for example.
   */
  template<typename Task>
  requires(NothrowInvocable<const Task&, std::size_t>)
  void execute(const Task& task, std::optional<std::size_t> used_thread_num = {}) const {
    const std::size_t used = used_thread_num.value_or(thread_num_);
    assert(used <= thread_num_);
    assert(std::this_thread::get_id() == owner_);

    if (used == 0) {
      return;
    }
    if (used == 1) {
      std::invoke(task, 0UZ);
      return;
    }

    task_fun_ = [](const void* data, std::size_t index) noexcept {
      std::invoke(*static_cast<std::remove_reference_t<const Task>*>(data), index);
    };
    task_data_ = std::addressof(task);
    // Ordering is enforced by later calls.
    unfinished_.store(used - 1, std::memory_order_relaxed);

    // The workers make their participation decision from this single load, so that those which are
    // not needed never touch the task, which the next region is free to overwrite.
    // The new state consists of the incremented epoch in the upper 48 bits and the current number
    // of threads used in the lower 16 bits.
    const u64 state = state_.load(std::memory_order_relaxed) + epoch_step;
    state_.store((state & ~used_mask) | used, std::memory_order_release);
    state_.notify_all();

    run(0);
    await_completion();
  }

private:
  /** The type of the function to be called on each thread. */
  using ThreadFun = void (*)(const void*, std::size_t) noexcept;

  /** The mask distinguishing the number of participating threads within the dispatch state. */
  static constexpr u64 used_mask = (1UZ << used_thread_bits) - 1;
  /** The number by which the dispatch state has to be increased to increment the epoch. */
  static constexpr u64 epoch_step = 1UZ << used_thread_bits;

  /**
   * A stand-in for the cache line size, which is an upper bound for current x86-64 CPUs (which
   * usually have 64-byte cache lines) and AArch64 CPUs (Apple Silicon has 128-byte cache lines).
   * Intel’s spatial prefetcher may load a pair of 64-byte cache lines, effectively making the
   * minimal unit for independent accesses 128 bytes there, too, as discussed here, for instance:
   *
   * github.com/crossbeam-rs/crossbeam/blob/main/crossbeam-utils/src/cache_padded.rs
   *
   * `std::hardware_destructive_interference_size` may not take this into account (it appears not to
   * on GCC and Clang on x86-64).
   */
  static constexpr std::size_t cache_line_bytes = 128;

  /**
   * The number of worker threads to create, which is `size - 1` for a non-empty pool.
   * Additionally, `size` is validated against `max_thread_num`.
   */
  static std::size_t worker_num(std::size_t size) {
    if (size > max_thread_num) {
      throw std::invalid_argument{
        cat(size, " threads have been requested, but at most ", max_thread_num, " are supported!"),
      };
    }
    return (size > 0) ? size - 1 : 0;
  }

  /** The loop run by every worker thread. */
  void work(std::size_t index) const {
    // Last iteration’s state to check whether new work has arrived.
    u64 last_state = 0;
    while (true) {
      last_state = await_state(last_state);
      if (stop_.load(std::memory_order_relaxed)) {
        break;
      }
      // Only run if the thread is used, which is encoded in the lower 16 bits of the state.
      if (index < (last_state & used_mask)) {
        run(index);
        // Notify the main thread once all threads are done.
        if (unfinished_.fetch_sub(1, std::memory_order_release) == 1) {
          unfinished_.notify_one();
        }
      }
    }
  }

  /**
   * Wait for a dispatch state other than `last`, spinning before parking.
   *
   * Similar to
   * https://github.com/gcc-mirror/gcc/blob/releases/gcc-15.2.0/libgomp/config/linux/wait.h#L48-L68.
   */
  u64 await_state(u64 last) const {
    for ([[maybe_unused]] const std::size_t i : views::countdown(spin_count_)) {
      const u64 state = state_.load(std::memory_order_acquire);
      if (state != last) {
        return state;
      }
      spin_pause();
    }
    while (true) {
      state_.wait(last, std::memory_order_acquire);
      const u64 state = state_.load(std::memory_order_acquire);
      if (state != last) {
        return state;
      }
    }
  }

  /** Wait for all participating workers to report completion, spinning before parking. */
  void await_completion() const {
    for ([[maybe_unused]] const std::size_t i : views::countdown(spin_count_)) {
      if (unfinished_.load(std::memory_order_acquire) == 0) {
        return;
      }
      spin_pause();
    }
    while (true) {
      const std::size_t left = unfinished_.load(std::memory_order_acquire);
      if (left == 0) {
        return;
      }
      unfinished_.wait(left, std::memory_order_acquire);
    }
  }

  /** Run the current task. `task_fun_` is `noexcept`, which this function inherits. */
  void run(std::size_t index) const noexcept {
    task_fun_(task_data_, index);
  }

  /**
   * The dispatch state that encodes the number of participating threads in its low 16 bits and the
   * current epoch in the upper 48 bits. Beyond being clever, this merged state also makes accesses
   * to both pieces of information, which are inherently tied together, one atomic operation,
   * avoiding race conditions.
   *
   * This approach is somewhat similar to how `val_` is packed in Folly’s `EventCount`:
   * github.com/facebook/folly/blob/v2026.09.21.00/folly/synchronization/EventCount.h
   */
  alignas(cache_line_bytes) mutable std::atomic<u64> state_{0};

  /** The total number of threads **including** the main thread. */
  std::size_t thread_num_;
  /** The number of spins before parking. */
  std::size_t spin_count_;
  /** The main thread that created this thread pool, to ensure that only it calls `execute`. */
  std::thread::id owner_ = std::this_thread::get_id();

  /** The atomic flag notifying threads when they should stop, i.e. when the pool is destroyed. */
  mutable std::atomic<bool> stop_{false};
  /**
   * The function to be executed on each thread, which re-interprets and calls `task_data_`.
   * Once the switch to C++26 is performed, `std::function_ref` can be used for equivalent
   * functionality.
   */
  mutable ThreadFun task_fun_ = nullptr;
  /** The data passed to `task_fun_`, which points to the actual callable. */
  mutable const void* task_data_ = nullptr;

  /**
   * The number of threads that are still working on the current epoch.
   * This is kept on a separate cache (and, on Intel, prefetch) line to avoid changes to it
   * invalidating other members, which is especially relevant with very short parallel sections.
   */
  alignas(cache_line_bytes) mutable std::atomic<std::size_t> unfinished_{0};

  /** The worker threads. */
  Threads workers_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_EXECUTOR_FIXED_THREAD_POOL_HPP
