// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_EXECUTION_SYSTEM_AFFINITY_HPP
#define INCLUDE_THESAUROS_EXECUTION_SYSTEM_AFFINITY_HPP

#include <cstddef>
#include <expected>
#include <thread>

#include <pthread.h>
#include <sched.h>

#include "thesauros/macropolis/platform.hpp"

#if THES_LINUX
#include <ranges>
#include <stdexcept>

#include "thesauros/ranges/iota.hpp"
#include "thesauros/utility/as-expected.hpp"
#elif THES_APPLE
#include <ranges>

#include <mach/mach.h>

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/math/integer-cast.hpp"
#include "thesauros/utility/as-expected.hpp"
#elif THES_WINDOWS
#include <memory>
#include <utility>

#include <errhandlingapi.h>
#include <minwindef.h>
#include <processthreadsapi.h>

#include "ankerl/unordered_dense.h"

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/math/integer-cast.hpp"
#endif

namespace thes {
struct CpuSet {
#if THES_LINUX
  using Id = std::size_t;
  using Base = cpu_set_t;
#elif THES_APPLE
  // The best alternative to thread pinning on macOS is to assign each thread
  // a different affinity tag
  // For this, we just store a single CPU, which will become the tag assigned to the thread
  // Sadly, this does not seem to work on M-series processors, as mentioned in
  // https://github.com/RenderKit/embree/blob/master/common/sys/thread.cpp
  using Id = integer_t;
  using Base = Id;
#elif THES_WINDOWS
  // With CPU Sets, the affinity is simply provided as an array of CPU IDs
  // To avoid duplicates, we use an unordered set which stores its values in an std::vector,
  // allowing for efficient access
  using Id = ULONG;
  using Base = ankerl::unordered_dense::set<Id>;
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
#elif THES_WINDOWS
  static CpuSet affinity(std::thread& thread) {
    // This implementation is just based around CPU sets; if the affinity was specified using
    // the older affinity API, this is (probably) not reflected here.

    auto* handle = pthread_gethandle(thread.native_handle());

    ULONG required_id_count{};
    GetThreadSelectedCpuSets(handle, nullptr, 0, &required_id_count);
    auto cpus = std::make_unique<ULONG>(required_id_count);
    ULONG size{};
    {
      const WINBOOL ret = GetThreadSelectedCpuSets(handle, cpus.get(), required_id_count, &size);
      if (ret != TRUE) {
        throw std::runtime_error{
          fmt::format("GetThreadSelectedCpuSets failed: {}", GetLastError())};
      }
    }
    return CpuSet{Base{cpus.get(), cpus.get() + size}};
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
#elif THES_WINDOWS
    cpu_set_.insert(*safe_cast<ULONG>(i));
#endif
  }

  [[nodiscard]] decltype(auto) cpu_ids() const {
#if THES_LINUX
    return range<std::size_t>(CPU_SETSIZE) |
           std::views::filter([&](std::size_t i) { return CPU_ISSET(i, &cpu_set_); });
#elif THES_APPLE
    return std::views::single(cpu_set_);
#else
    return cpu_set_;
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
#elif THES_WINDOWS
  Base cpu_set_{};

  explicit CpuSet(Base&& cpu_set) : cpu_set_{std::move(cpu_set)} {}
#endif
};

#if THES_LINUX
inline std::expected<void, int> set_affinity(std::thread::native_handle_type handle,
                                             const CpuSet& cpu_set) {
  const auto ret = pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpu_set.base());
  return as_expected(ret);
}
#elif THES_APPLE
inline std::expected<void, kern_return_t> set_affinity(std::thread::native_handle_type handle,
                                                       const CpuSet& cpu_set) {
  // based on https://www.hybridkernel.com/2015/01/18/binding_threads_to_cores_osx.html
  // and https://developer.apple.com/library/archive/releasenotes/Performance/RN-AffinityAPI/
  const auto mach_thread = pthread_mach_thread_np(handle);
  thread_affinity_policy_data_t policy{cpu_set.base()};
  const kern_return_t ret = thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY,
                                              reinterpret_cast<thread_policy_t>(&policy), 1);
  return as_expected(ret);
}
#else
inline std::expected<void, WINBOOL> set_affinity(std::thread::native_handle_type handle,
                                                 const CpuSet& cpu_set) {
  // Windows has two APIs to change thread affinity, as discussed in
  // https://stackoverflow.com/q/76317127:
  // - An older one using `SetThreadAffinityMask`/`SetThreadGroupAffinity`, which can only set
  //   the affinity within the 64-core “group” that the thread is assigned to, as discussed on
  //   https://learn.microsoft.com/en-us/windows/win32/procthread/processor-groups.
  //   The group assignment can be changed, but cross-group affinity is still not possible.
  //   This is not particularly problematic in many cases, in which each thread is pinned
  //   to a single core, but handling the general case would be very annoying and require some
  //   compromises.
  // - A newer one using `SetThreadSelectedCpuSets`, which is apparently a softer way to specify
  //   thread affinity, as discussed on
  //   https://learn.microsoft.com/en-us/windows/win32/procthread/cpu-sets.
  //   However, this allows specifying affinity across groups, which is more flexible and does not
  //   require weird compromises, and the cases in which this ignores the provided affinities seem
  //   limited and reasonable.
  const WINBOOL ret = SetThreadSelectedCpuSets(
    pthread_gethandle(handle), cpu_set.base().values().data(), cpu_set.base().size());
  if (ret == 0) {
    return std::unexpected(ret);
  }
  return {};
}
#endif
inline auto set_affinity(std::thread& thread, const CpuSet& cpu_set) {
  return set_affinity(thread.native_handle(), cpu_set);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_SYSTEM_AFFINITY_HPP
