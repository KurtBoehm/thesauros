#ifndef INCLUDE_THESAUROS_EXECUTION_EXECUTOR_FIXED_THREAD_POOL_HPP
#define INCLUDE_THESAUROS_EXECUTION_EXECUTOR_FIXED_THREAD_POOL_HPP

#include <cassert>
#include <concepts>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>

#include "thesauros/containers/array/fixed-alloc.hpp"
#include "thesauros/execution/system.hpp"
#include "thesauros/execution/system/affinity.hpp"
#include "thesauros/ranges/iota.hpp"
#include "thesauros/utility/empty.hpp"

namespace thes {
struct FixedThreadPool {
  using Threads = FixedAllocArray<std::thread>;
  using Task = std::function<void(std::size_t)>;
  using TaskID = std::size_t;

  template<typename TCpuSets = Empty>
  explicit FixedThreadPool(std::size_t size, const TCpuSets& cpu_sets = {})
      : threads_(Threads::create_with_capacity(size)) {
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

          task(i);
          lock.lock();
          --unfinished_;
          lock.unlock();
          end_work_.notify_all();
        }
      });
      if constexpr (std::same_as<TCpuSets, Empty>) {
        set_affinity(threads_[i], CPUSet::single_set(i));
      } else {
        set_affinity(threads_[i], cpu_sets[i]);
      }
      set_scheduler(threads_[i], Scheduler::FIFO);
    }
  }

  FixedThreadPool(const FixedThreadPool&) = delete;
  FixedThreadPool(FixedThreadPool&&) = delete;
  FixedThreadPool& operator=(const FixedThreadPool&) = delete;
  FixedThreadPool& operator=(FixedThreadPool&&) = delete;

  ~FixedThreadPool() {
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

  [[nodiscard]] std::size_t size() const {
    return threads_.size();
  }

  void execute(Task task) const {
    {
      const std::lock_guard lock{work_mutex_};
      task_ = std::move(task);
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

#endif // INCLUDE_THESAUROS_EXECUTION_EXECUTOR_FIXED_THREAD_POOL_HPP
