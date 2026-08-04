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
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <thread>
#include <utility>

#include <pthread.h>

#include "thesauros/charconv/concat.hpp"
#include "thesauros/containers/array/fixed-alloc.hpp"
#include "thesauros/execution/system/affinity.hpp"
#include "thesauros/execution/system/spin.hpp"
#include "thesauros/math/integer-cast.hpp"
#include "thesauros/ranges/index-type.hpp"
#include "thesauros/ranges/indices.hpp"
#include "thesauros/types/empty.hpp"

namespace thes {
/**
 * A thread pool of a fixed size which aims to make the dispatch of a parallel region as cheap as
 * an OpenMP one while relying on nothing but the standard library.
 *
 * The three properties that matter for small per-thread workloads are:
 * - The calling thread executes index 0 instead of blocking, which removes a pair of context
 *   switches per region and avoids oversubscribing the machine by one thread.
 * - The task is passed as a function pointer plus a pointer to the callable, which lives on the
 *   caller’s stack for the duration of the blocking `execute`, so nothing is allocated or copied.
 * - Threads spin for `spin_count` iterations before parking on `std::atomic::wait`, which maps to
 *   `futex` on Linux, `__ulock_wait` on macOS and `WaitOnAddress` on Windows. Back-to-back regions
 *   therefore never enter the kernel, while a pool left idle stops consuming CPU time.
 *
 * Unlike an OpenMP parallel region, an exception escaping a task is not fatal: the first one is
 * captured and rethrown from `execute` on the calling thread.
 *
 * `execute` must only be called from the thread that created the pool and never from within a
 * task, which is checked by an assertion.
 */
struct FixedThreadPool {
  /**
   * The number of iterations spent spinning before parking, which trades the latency of the next
   * region against the CPU time burnt while waiting for it. The default corresponds to a few tens
   * of microseconds, in the spirit of libgomp’s `GOMP_SPINCOUNT`; zero parks immediately.
   */
  static constexpr std::size_t default_spin_count = std::size_t{1} << 14U;
  /** The largest supported pool size, imposed by the packing of the dispatch state. */
  static constexpr std::size_t max_thread_num = (std::size_t{1} << 16U) - 1;

  using Threads = FixedAllocArray<std::jthread>;

  /**
   * Create a pool of `size` threads, of which the calling thread is one.
   * @param cpu_sets The CPU sets to pin the threads to, with the first entry applying to the
   *                 calling thread, or `Empty` to leave the affinities alone.
   */
  template<typename CpuSets = Empty>
  explicit FixedThreadPool(std::size_t size, const CpuSets& cpu_sets = {},
                           std::size_t spin_count = default_spin_count)
      : thread_num_{size}, spin_count_{spin_count},
        threads_{Threads::create_with_capacity(worker_num(size))} {
    if constexpr (!std::same_as<CpuSets, Empty>) {
      if (size > cpu_sets.size()) {
        throw std::invalid_argument{cat(size, " threads have been requested, but there are only ",
                                        cpu_sets.size(), " entries in the CPU set!")};
      }
    }

    const std::size_t workers = worker_num(size);
    for (const std::size_t i : views::indices(workers)) {
      threads_.emplace_back([this, index = i + 1] { work(index); });
    }

    if constexpr (!std::same_as<CpuSets, Empty>) {
      using Index = ranges::RangeIndex<CpuSets>;
      if (size > 0) {
        (void)set_affinity(pthread_self(), cpu_sets[Index{0}]);
      }
      for (const std::size_t i : views::indices(workers)) {
        (void)set_affinity(threads_[i], cpu_sets[*thes::safe_cast<Index>(i + 1)]);
      }
    }
  }

  /** Create a pool pinning each thread to one of the CPUs described by `cpu_infos`. */
  template<typename CpuInfos = Empty>
  static FixedThreadPool from_cpu_infos(std::size_t size, CpuInfos&& cpu_infos = {},
                                        std::size_t spin_count = default_spin_count) {
    if constexpr (std::same_as<CpuInfos, Empty>) {
      return FixedThreadPool{size, Empty{}, spin_count};
    } else {
      return FixedThreadPool{
        size,
        std::views::transform(std::forward<CpuInfos>(cpu_infos),
                              [](auto cpu) { return CpuSet::single_set(cpu.id); }),
        spin_count};
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
   * Run `task` on the thread indices `[0, used_thread_num)`, blocking until all of them are done,
   * and rethrow the first exception any of them produced.
   */
  template<typename Task>
  requires(std::invocable<const Task&, std::size_t>)
  void execute(const Task& task, std::optional<std::size_t> used_thread_num = {}) const {
    const std::size_t used = used_thread_num.value_or(thread_num_);
    assert(used <= thread_num_);
    assert(std::this_thread::get_id() == owner_);

    if (used == 0) {
      return;
    }
    if (used == 1) {
      std::invoke(task, std::size_t{0});
      return;
    }

    task_fun_ = [](const void* data, std::size_t index) {
      std::invoke(*static_cast<const Task*>(data), index);
    };
    task_data_ = std::addressof(task);
    unfinished_.store(used - 1, std::memory_order_relaxed);

    // The workers make their participation decision from this single load, so that those which are
    // not needed never touch the task, which the next region is free to overwrite.
    const std::size_t state = state_.load(std::memory_order_relaxed) + epoch_step;
    state_.store((state & ~used_mask) | used, std::memory_order_release);
    state_.notify_all();

    run(0);
    await_completion();

    if (exception_stored_.exchange(false, std::memory_order_acquire)) {
      std::rethrow_exception(std::exchange(exception_, {}));
    }
  }

private:
  // The dispatch state packs the number of participating threads into the low bits and a counter
  // of the regions dispatched so far into the high bits.
  static constexpr std::size_t used_mask = max_thread_num;
  static constexpr std::size_t epoch_step = max_thread_num + 1;
  // Keeping the two hot atomics on separate cache lines prevents the completion counter, which
  // every worker modifies, from invalidating the line the workers spin on.
  static constexpr std::size_t cache_line_bytes = 128;

  /**
   * The number of threads to create, i.e. all but the calling one, validating `size` on the way.
   */
  static std::size_t worker_num(std::size_t size) {
    if (size > max_thread_num) {
      throw std::invalid_argument{
        cat(size, " threads have been requested, but at most ", max_thread_num, " are supported!")};
    }
    return (size > 0) ? size - 1 : 0;
  }

  /** The loop run by every thread but the calling one. */
  void work(std::size_t index) const {
    std::size_t last_state = 0;
    while (true) {
      last_state = await_state(last_state);
      if (stop_.load(std::memory_order_relaxed)) {
        break;
      }
      if (index < (last_state & used_mask)) {
        run(index);
        if (unfinished_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
          unfinished_.notify_one();
        }
      }
    }
  }

  /** Wait for a dispatch state other than `last`, spinning before parking. */
  std::size_t await_state(std::size_t last) const {
    for (std::size_t i = spin_count_; i > 0; --i) {
      const std::size_t state = state_.load(std::memory_order_acquire);
      if (state != last) {
        return state;
      }
      spin_pause();
    }
    while (true) {
      state_.wait(last, std::memory_order_acquire);
      const std::size_t state = state_.load(std::memory_order_acquire);
      if (state != last) {
        return state;
      }
    }
  }

  /** Wait for all participating workers to report completion, spinning before parking. */
  void await_completion() const {
    for (std::size_t i = spin_count_; i > 0; --i) {
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

  /** Run the current task, storing the exception it throws if it is the first one. */
  void run(std::size_t index) const {
    try {
      task_fun_(task_data_, index);
    } catch (...) {
      if (!exception_stored_.exchange(true, std::memory_order_acq_rel)) {
        exception_ = std::current_exception();
      }
    }
  }

  std::size_t thread_num_;
  std::size_t spin_count_;
  std::thread::id owner_{std::this_thread::get_id()};

  // Written by the calling thread before the release store to `state_` and read by the workers
  // that the store makes participants, which the calling thread waits for before writing again.
  alignas(cache_line_bytes) mutable std::atomic<std::size_t> state_{0};
  mutable std::atomic<bool> stop_{false};
  mutable void (*task_fun_)(const void*, std::size_t){nullptr};
  mutable const void* task_data_{nullptr};

  alignas(cache_line_bytes) mutable std::atomic<std::size_t> unfinished_{0};

  mutable std::atomic<bool> exception_stored_{false};
  mutable std::exception_ptr exception_{};

  Threads threads_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_EXECUTOR_FIXED_THREAD_POOL_HPP
