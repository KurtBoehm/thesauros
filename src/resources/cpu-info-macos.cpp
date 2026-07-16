// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <stdexcept>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

#include "thesauros/resources/cpu-info.hpp"

// The approach is based on https://github.com/open-mpi/hwloc/blob/master/hwloc/topology-darwin.c,
// but heavily modernized and somewhat optimized.

namespace {
//--------------------------------------------------------------------------------------------------
// A generic RAII handle for release-function-style resources
//--------------------------------------------------------------------------------------------------

/** A move-only RAII wrapper around a handle type `T` released via `Release`. */
template<typename T, auto Release, T Invalid = T{}>
struct UniqueHandle {
  UniqueHandle() = default;

  explicit UniqueHandle(T value) : value_{value} {}

  UniqueHandle(const UniqueHandle&) = delete;
  UniqueHandle& operator=(const UniqueHandle&) = delete;

  UniqueHandle(UniqueHandle&& other) noexcept : value_{other.value_} {
    other.value_ = Invalid;
  }

  UniqueHandle& operator=(UniqueHandle&& other) noexcept {
    if (this != &other) {
      reset();
      value_ = other.value_;
      other.value_ = Invalid;
    }
    return *this;
  }

  ~UniqueHandle() {
    reset();
  }

  /** Releases the current handle, if any, and replaces it with `new_value`. */
  void reset(T new_value = Invalid) {
    if (value_ != Invalid) {
      Release(value_);
    }
    value_ = new_value;
  }

  [[nodiscard]] T get() const {
    return value_;
  }

  [[nodiscard]] bool has_value() const {
    return value_ != Invalid;
  }

private:
  T value_ = Invalid;
};

using CfRef = UniqueHandle<CFTypeRef, CFRelease, nullptr>;
using IoObject = UniqueHandle<io_object_t, IOObjectRelease, io_object_t{0}>;

/** Casts a `CfRef`’s underlying pointer to a more specific `CFTypeRef` subtype. */
template<typename T>
[[nodiscard]] T cf_as(const CfRef& ref) {
  return static_cast<T>(ref.get());
}

//--------------------------------------------------------------------------------------------------
// Topology helpers
//--------------------------------------------------------------------------------------------------

/** Maps a “cluster-type” byte to an efficiency class. */
inline thes::EfficiencyClass cluster_type_to_efficiency_class(thes::u8 cluster_type) {
  using enum thes::EfficiencyClass;
  switch (cluster_type) {
    case 'E': return efficiency;
    case 'M': return medium;
    case 'P': return performance;
    default: return any;
  }
}
} // namespace

std::vector<thes::detail::CpuEntry> thes::detail::compute_cpu_topology() {
  const auto logical_cores = *safe_cast<std::size_t>(read_sysctl<int>("hw.logicalcpu"));
  if (logical_cores == 0) {
    throw std::runtime_error{"No logical cores detected"};
  }

  constexpr char dt_plane_name[] = "IODeviceTree";
  constexpr char cpu_plane_path[] = "IODeviceTree:/cpus";

  IoObject cpus_root{IORegistryEntryFromPath(kIOMainPortDefault, cpu_plane_path)};
  if (!cpus_root.has_value()) {
    throw std::runtime_error{"No registry entry at IODeviceTree:/cpus found"};
  }

  io_iterator_t raw_iter{};
  if (IORegistryEntryGetChildIterator(cpus_root.get(), dt_plane_name, &raw_iter) != KERN_SUCCESS) {
    throw std::runtime_error{"Getting children of IODeviceTree:/cpus failed"};
  }
  IoObject cpus_iter{raw_iter};

  std::vector<CpuEntry> entries{};
  for (IoObject cpus_child{IOIteratorNext(cpus_iter.get())}; cpus_child.has_value();
       cpus_child.reset(IOIteratorNext(cpus_iter.get()))) {
    CfRef logical_id_ref{IORegistryEntrySearchCFProperty(
      cpus_child.get(), dt_plane_name, CFSTR("logical-cpu-id"), kCFAllocatorDefault, kNilOptions)};
    if (!logical_id_ref.has_value() || CFGetTypeID(logical_id_ref.get()) != CFNumberGetTypeID()) {
      continue;
    }

    long long logical_id_ll{};
    const bool ok = CFNumberGetValue(cf_as<CFNumberRef>(logical_id_ref), kCFNumberLongLongType,
                                     &logical_id_ll) != 0;
    if (!ok || logical_id_ll < 0) {
      continue;
    }

    const auto logical_id = *safe_cast<std::size_t>(logical_id_ll);
    if (logical_id >= logical_cores) {
      continue;
    }

    EfficiencyClass efficiency_class = EfficiencyClass::any;
    CfRef cluster_ref{IORegistryEntrySearchCFProperty(
      cpus_child.get(), dt_plane_name, CFSTR("cluster-type"), kCFAllocatorDefault, kNilOptions)};
    if (cluster_ref.has_value() && CFGetTypeID(cluster_ref.get()) == CFDataGetTypeID()) {
      const auto* const data_ref = cf_as<CFDataRef>(cluster_ref);
      const CFIndex length = CFDataGetLength(data_ref);
      if (length >= 1) {
        std::array<UInt8, 2> buffer{0, 0};
        CFDataGetBytes(data_ref, CFRangeMake(0, std::min<CFIndex>(length, 2)), buffer.data());
        efficiency_class = cluster_type_to_efficiency_class(buffer[0]);
      }
    }

    entries.push_back({.logical_id = logical_id, .efficiency_class = efficiency_class});
  }

  if (entries.empty()) {
    throw std::runtime_error{"No CPUs found under IODeviceTree:/cpus"};
  }

  std::ranges::sort(entries, {}, &CpuEntry::logical_id);

  if (entries.size() != logical_cores) {
    throw std::runtime_error{
      "The number of CPUs under IODeviceTree:/cpus does not match the number of logical CPUs"};
  }
  for (std::size_t i = 0; i < entries.size(); ++i) {
    if (entries[i].logical_id != i) {
      throw std::runtime_error{"The logical ID of a CPU does not match its index"};
    }
  }

  return entries;
}
