// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_RESOURCES_RESOURCE_USAGE_HPP
#define INCLUDE_THESAUROS_RESOURCES_RESOURCE_USAGE_HPP

#include "thesauros/macropolis/platform.hpp"
#include "thesauros/quantity/quantity.hpp"

#if THES_LINUX || THES_APPLE
#include <sys/resource.h>
#elif THES_WINDOWS
#include <stdexcept>

#include <errhandlingapi.h>
#include <minwindef.h>
#include <processthreadsapi.h>
#include <psapi.h>
#include <winnt.h>

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/types/primitives.hpp"
#endif

namespace thes {
#if THES_LINUX || THES_APPLE
struct ResourceUsage {
  using Duration = Quantity<long, unit::microsecond>;
  using Memory = Quantity<long, unit::kibibyte>;

  ResourceUsage() : ResourceUsage(get_rusage()) {}

  [[nodiscard]] Memory max_memory() const {
    return max_memory_;
  }
  [[nodiscard]] Duration user_time() const {
    return user_time_;
  }
  [[nodiscard]] Duration system_time() const {
    return system_time_;
  }

private:
  static rusage get_rusage() {
    rusage usage{};
    getrusage(RUSAGE_SELF, &usage);
    return usage;
  }

  static Duration convert_time(const timeval time) {
    const auto sec = Quantity<long, unit::second>(time.tv_sec);
    const auto usec = Quantity<long, unit::microsecond>(time.tv_usec);
    return sec + usec;
  }

  explicit ResourceUsage(rusage usage)
      : max_memory_(usage.ru_maxrss), user_time_(convert_time(usage.ru_utime)),
        system_time_(convert_time(usage.ru_stime)) {}

  Memory max_memory_;
  Duration user_time_;
  Duration system_time_;
};
#else
struct ResourceUsage {
  using Duration = Quantity<u64, unit::microsecond>;
  using Memory = Quantity<SIZE_T, unit::byte>;

  ResourceUsage() : ResourceUsage(get_stats()) {}

  [[nodiscard]] Memory max_memory() const {
    return max_memory_;
  }
  [[nodiscard]] Duration user_time() const {
    return user_time_;
  }
  [[nodiscard]] Duration system_time() const {
    return system_time_;
  }

private:
  struct SystemStats {
    FILETIME kernel_time;
    FILETIME user_time;
    PROCESS_MEMORY_COUNTERS mem_counters;
  };

  static SystemStats get_stats() {
    // inspired by https://github.com/openvswitch/ovs/blob/main/lib/getrusage-windows.c
    // and https://github.com/postgres/postgres/blob/master/src/port/win32getrusage.c
    FILETIME kernel_time{};
    FILETIME user_time{};
    {
      FILETIME creation_time{};
      FILETIME exit_time{};
      const WINBOOL ret =
        GetProcessTimes(GetCurrentProcess(), &creation_time, &exit_time, &kernel_time, &user_time);
      if (ret == 0) {
        throw std::runtime_error{fmt::format("GetProcessTimes failed: {}", GetLastError())};
      }
    }

    PROCESS_MEMORY_COUNTERS mem_counters{};
    {
      const WINBOOL ret =
        GetProcessMemoryInfo(GetCurrentProcess(), &mem_counters, sizeof(mem_counters));
      if (ret == 0) {
        throw std::runtime_error{fmt::format("GetProcessMemoryInfo failed: {}", GetLastError())};
      }
    }

    return {.kernel_time = kernel_time, .user_time = user_time, .mem_counters = mem_counters};
  }

  static Duration convert_time(const FILETIME time) {
    // the unit is 100ns
    const auto raw = (u64(time.dwHighDateTime) << 32U) | u64(time.dwLowDateTime);
    return Quantity<u64, unit::microsecond>(raw / 10);
  }

  explicit ResourceUsage(SystemStats stats)
      : max_memory_(stats.mem_counters.PeakWorkingSetSize),
        user_time_(convert_time(stats.user_time)), system_time_(convert_time(stats.kernel_time)) {}

  Memory max_memory_;
  Duration user_time_;
  Duration system_time_;
};
#endif
} // namespace thes

#endif // INCLUDE_THESAUROS_RESOURCES_RESOURCE_USAGE_HPP
