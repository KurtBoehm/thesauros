// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_EXECUTION_SYSTEM_AFFINITY_HPP
#define INCLUDE_THESAUROS_EXECUTION_SYSTEM_AFFINITY_HPP

#include <cstddef>
#include <expected>
#include <ranges>
#include <stdexcept>
#include <thread>

#include <pthread.h>
#include <sched.h>

#include "thesauros/ranges/iota.hpp"
#include "thesauros/utility/as-expected.hpp"

namespace thes {
struct CpuSet {
  CpuSet() {
    CPU_ZERO(&cpu_set_);
  }

  CpuSet(const CpuSet& other) = delete;
  CpuSet(CpuSet&& other) = default;
  CpuSet& operator=(const CpuSet& other) = delete;
  CpuSet& operator=(CpuSet&& other) = default;

  ~CpuSet() = default;

  static CpuSet single_set(std::size_t i) {
    CpuSet output{};
    output.set(i);
    return output;
  }
  static CpuSet affinity(std::thread& thread) {
    CpuSet out{};
    const auto ret =
      pthread_getaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &out.cpu_set_);
    if (ret != 0) {
      throw std::runtime_error{"pthread_getaffinity_np failed!"};
    }
    return out;
  }
  static CpuSet from_cpu_infos(const auto& infos) {
    CpuSet output{};
    for (const auto& cpu : infos) {
      output.set(cpu.id);
    }
    return output;
  }

  void set(std::size_t i) {
    CPU_SET(i, &cpu_set_);
  }

  [[nodiscard]] auto cpus() const {
    return range<std::size_t>(CPU_SETSIZE) |
           std::views::filter([&](std::size_t i) { return CPU_ISSET(i, &cpu_set_); });
  }

  [[nodiscard]] const cpu_set_t& base() const {
    return cpu_set_;
  }

private:
  cpu_set_t cpu_set_{};
};

inline std::expected<void, int> set_affinity(std::thread& thread, const CpuSet& cpu_set) {
  const auto ret =
    pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &cpu_set.base());
  return as_expected(ret);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_SYSTEM_AFFINITY_HPP
