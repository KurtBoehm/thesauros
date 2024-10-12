#ifndef INCLUDE_THESAUROS_EXECUTION_EXECUTOR_FIXED_OMP_THREAD_POOL_HPP
#define INCLUDE_THESAUROS_EXECUTION_EXECUTOR_FIXED_OMP_THREAD_POOL_HPP

#include <cassert>
#include <concepts>
#include <cstddef>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

#include <pthread.h>
#include <sched.h>

#include "thesauros/execution/system.hpp"
#include "thesauros/execution/system/affinity.hpp"
#include "thesauros/format/fmtlib.hpp"
#include "thesauros/utility/empty.hpp"

namespace thes {
struct FixedOpenMpThreadPool {
  template<typename TCpuSets = Empty>
  explicit FixedOpenMpThreadPool(std::size_t size, TCpuSets cpu_sets = {}) : thread_num_(size) {
    if constexpr (!std::same_as<TCpuSets, Empty>) {
      if (size > cpu_sets.size()) {
        throw std::invalid_argument{::fmt::format("{} threads have been requested, "
                                                  "but there are only {} entries in the CPU set!",
                                                  size, cpu_sets.size())};
      }
      auto rng = std::views::common(std::move(cpu_sets));
      cpu_sets_.emplace(rng.begin(), rng.end());
    }
  }

  template<typename TCpuInfos = Empty>
  static FixedOpenMpThreadPool from_cpu_infos(std::size_t size, TCpuInfos&& cpu_infos = {}) {
    if constexpr (std::same_as<TCpuInfos, Empty>) {
      return FixedOpenMpThreadPool{size, Empty{}};
    } else {
      return FixedOpenMpThreadPool{
        size, std::views::transform(std::forward<TCpuInfos>(cpu_infos),
                                    [](auto cpu) { return CpuSet::single_set(cpu.id); })};
    }
  }

  FixedOpenMpThreadPool(const FixedOpenMpThreadPool&) = delete;
  FixedOpenMpThreadPool(FixedOpenMpThreadPool&&) = delete;
  FixedOpenMpThreadPool& operator=(const FixedOpenMpThreadPool&) = delete;
  FixedOpenMpThreadPool& operator=(FixedOpenMpThreadPool&&) = delete;

  ~FixedOpenMpThreadPool() = default;

  [[nodiscard]] std::size_t thread_num() const {
    return thread_num_;
  }

  template<std::invocable<std::size_t> TTask>
  void execute(TTask task) const {
#pragma omp parallel for num_threads(thread_num_)
    for (std::size_t t = 0; t < thread_num_; ++t) {
      if (cpu_sets_.has_value()) {
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &(*cpu_sets_)[t].base());
      }
      task(t);
    }
  }

private:
  std::size_t thread_num_;
  std::optional<std::vector<CpuSet>> cpu_sets_{};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_EXECUTOR_FIXED_OMP_THREAD_POOL_HPP
