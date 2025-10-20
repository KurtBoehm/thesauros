// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

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

#include <omp.h>
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
        throw std::invalid_argument{fmt::format("{} threads have been requested, "
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

  void execute(std::invocable<std::size_t> auto task,
               std::optional<std::size_t> used_thread_num = {}) const {
    assert(!used_thread_num.has_value() || *used_thread_num <= thread_num_);
    const std::size_t tnum = used_thread_num.value_or(thread_num_);
    if (omp_get_max_threads() < thread_num_) {
      throw std::runtime_error{fmt::format("The thread pool needs {} threads, but the max is {}",
                                           thread_num_, omp_get_max_threads())};
    }
#pragma omp parallel for num_threads(thread_num_)
    for (std::size_t t = 0; t < tnum; ++t) {
      if (cpu_sets_.has_value()) {
        (void)set_affinity(pthread_self(), (*cpu_sets_)[t]);
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
