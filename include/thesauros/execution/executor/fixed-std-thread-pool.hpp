// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_EXECUTION_EXECUTOR_FIXED_STD_THREAD_POOL_HPP
#define INCLUDE_THESAUROS_EXECUTION_EXECUTOR_FIXED_STD_THREAD_POOL_HPP

#include <cassert>
#include <concepts>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <thread>
#include <utility>

#include "thesauros/containers/array/fixed-alloc.hpp"
#include "thesauros/execution/system.hpp"
#include "thesauros/execution/system/affinity.hpp"
#include "thesauros/format/fmtlib.hpp"
#include "thesauros/ranges/iota.hpp"
#include "thesauros/utility/empty.hpp"

namespace thes {
struct FixedStdThreadPool {
  using Op = std::function<void(std::size_t)>;
  struct Task {
    Op fun;
    std::optional<std::size_t> used_thread_num;
  };
  using Threads = FixedAllocArray<std::thread>;
  using TaskID = std::size_t;

  template<typename TCpuSets = Empty>
  explicit FixedStdThreadPool(std::size_t size, const TCpuSets& cpu_sets = {})
      : threads_(Threads::create_with_capacity(size)) {
    if constexpr (!std::same_as<TCpuSets, Empty>) {
      if (size > cpu_sets.size()) {
        throw std::invalid_argument{fmt::format("{} threads have been requested, "
                                                "but there are only {} entries in the CPU set!",
                                                size, cpu_sets.size())};
      }
    }

    for (const std::size_t i : range(size)) {
      threads_.emplace_back([this, i] {
        TaskID last_id{0};

        while (true) {
          std::unique_lock lock{work_mutex_};
          start_work_.wait(lock, [this, last_id] { return task_id_ != last_id; });
          if (!task_.has_value()) {
            break;
          }

          const Task task{task_.value()};
          last_id = task_id_;
          lock.unlock();

          if (!task.used_thread_num.has_value() || i < *task.used_thread_num) {
            task.fun(i);
          }
          lock.lock();
          --unfinished_;
          lock.unlock();
          end_work_.notify_all();
        }
      });
      if constexpr (!std::same_as<TCpuSets, Empty>) {
        (void)set_affinity(threads_[i], cpu_sets[i]);
      }
    }
  }

  template<typename TCpuInfos = Empty>
  static FixedStdThreadPool from_cpu_infos(std::size_t size, TCpuInfos&& cpu_infos = {}) {
    if constexpr (std::same_as<TCpuInfos, Empty>) {
      return FixedStdThreadPool{size, Empty{}};
    } else {
      return FixedStdThreadPool{
        size, std::views::transform(std::forward<TCpuInfos>(cpu_infos),
                                    [](auto cpu) { return CpuSet::single_set(cpu.id); })};
    }
  }

  FixedStdThreadPool(const FixedStdThreadPool&) = delete;
  FixedStdThreadPool(FixedStdThreadPool&&) = delete;
  FixedStdThreadPool& operator=(const FixedStdThreadPool&) = delete;
  FixedStdThreadPool& operator=(FixedStdThreadPool&&) = delete;

  ~FixedStdThreadPool() {
    {
      const std::lock_guard lock{work_mutex_};
      assert(!task_.has_value());
      ++task_id_;
    }
    start_work_.notify_all();
    for (std::thread& t : threads_) {
      t.join();
    }
  }

  [[nodiscard]] std::size_t thread_num() const {
    return threads_.size();
  }

  void execute(Op task, std::optional<std::size_t> used_thread_num = {}) const {
    assert(!used_thread_num.has_value() || *used_thread_num <= threads_.size());
    {
      const std::lock_guard lock{work_mutex_};
      task_.emplace(std::move(task), used_thread_num);
      unfinished_ = threads_.size();
      ++task_id_;
    }
    start_work_.notify_all();
    {
      std::unique_lock lock{work_mutex_};
      end_work_.wait(lock, [&unfinished = unfinished_] { return unfinished == 0; });
      task_.reset();
    }
  }

private:
  mutable TaskID task_id_{0};
  mutable std::optional<Task> task_{};
  mutable std::size_t unfinished_{0};

  mutable std::mutex work_mutex_{};
  mutable std::condition_variable start_work_{};
  mutable std::condition_variable end_work_{};

  Threads threads_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_EXECUTOR_FIXED_STD_THREAD_POOL_HPP
