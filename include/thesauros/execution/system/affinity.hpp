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
#include <thread>

#include <pthread.h>
#include <sched.h>

#include "thesauros/macropolis/platform.hpp"
#include "thesauros/utility/as-expected.hpp"

#if THES_LINUX
#include <stdexcept>

#include "thesauros/ranges/iota.hpp"
#elif THES_APPLE
#include <mach/mach.h>

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/math/integer-cast.hpp"
#endif

namespace thes {
struct CpuSet {
#if THES_LINUX
  using Base = cpu_set_t;
#elif THES_APPLE
  // The best alternative to thread pinning on macOS is to assign each thread
  // a different affinity tag
  // For this, we just store a single CPU, which will become the tag assigned to the thread
  // Sadly, this does not seem to work on M-series processors, as mentioned in
  // https://github.com/RenderKit/embree/blob/master/common/sys/thread.cpp
  using Base = integer_t;
#endif

  CpuSet() {
#if THES_LINUX
    CPU_ZERO(&cpu_set_);
#endif
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
  // There does not seem to be an equivalent on macOS
#if THES_LINUX
  static CpuSet affinity(std::thread& thread) {
    CpuSet out{};
    const auto ret =
      pthread_getaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &out.cpu_set_);
    if (ret != 0) {
      throw std::runtime_error{"pthread_getaffinity_np failed!"};
    }
    return out;
  }
#endif
  static CpuSet from_cpu_infos(const auto& infos) {
    CpuSet output{};
    for (const auto& cpu : infos) {
      output.set(cpu.id);
    }
    return output;
  }

  void set(std::size_t i) {
#if THES_LINUX
    CPU_SET(i, &cpu_set_);
#elif THES_APPLE
    if (cpu_set_ != integer_t(-1)) {
      fmt::print(stderr, "On macOS, multi-CPU sets are not supported! Overwriting existing entry.");
    }
    cpu_set_ = *safe_cast<Base>(i);
#endif
  }

  [[nodiscard]] auto cpus() const {
#if THES_LINUX
    return range<std::size_t>(CPU_SETSIZE) |
           std::views::filter([&](std::size_t i) { return CPU_ISSET(i, &cpu_set_); });
#elif THES_APPLE
    return std::views::single(cpu_set_);
#endif
  }

  [[nodiscard]] const Base& base() const {
    return cpu_set_;
  }

private:
#if THES_LINUX
  Base cpu_set_{};
#elif THES_APPLE
  Base cpu_set_ = -1;
#endif
};

inline std::expected<void, int> set_affinity(std::thread::native_handle_type handle,
                                             const CpuSet& cpu_set) {
#if THES_LINUX
  const auto ret = pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpu_set.base());
#elif THES_APPLE
  // based on https://www.hybridkernel.com/2015/01/18/binding_threads_to_cores_osx.html
  // and https://developer.apple.com/library/archive/releasenotes/Performance/RN-AffinityAPI/
  const auto mach_thread = pthread_mach_thread_np(handle);
  thread_affinity_policy_data_t policy{cpu_set.base()};
  const kern_return_t ret = thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY,
                                              reinterpret_cast<thread_policy_t>(&policy), 1);
#endif
  return as_expected(ret);
}
inline std::expected<void, int> set_affinity(std::thread& thread, const CpuSet& cpu_set) {
  return set_affinity(thread.native_handle(), cpu_set);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_SYSTEM_AFFINITY_HPP
