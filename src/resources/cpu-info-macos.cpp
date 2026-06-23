#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

#include "thesauros/resources/cpu-info.hpp"

namespace {
//--------------------------------------------------------------------------------------------------
// RAII wrappers for CoreFoundation and IOKit
//--------------------------------------------------------------------------------------------------

/** RAII wrapper for `CFTypeRef`. */
struct CfRef {
  CfRef() = default;

  explicit CfRef(CFTypeRef ptr) : ptr_{ptr} {}

  CfRef(const CfRef&) = delete;
  CfRef& operator=(const CfRef&) = delete;

  CfRef(CfRef&& other) noexcept : ptr_{other.ptr_} {
    other.ptr_ = nullptr;
  }

  CfRef& operator=(CfRef&& other) noexcept {
    if (this != &other) {
      reset();
      ptr_ = other.ptr_;
      other.ptr_ = nullptr;
    }
    return *this;
  }

  ~CfRef() {
    reset();
  }

  void reset(CFTypeRef new_ptr = nullptr) {
    if (ptr_ != nullptr) {
      CFRelease(ptr_);
    }
    ptr_ = new_ptr;
  }

  [[nodiscard]] CFTypeRef get() const {
    return ptr_;
  }

  template<typename T>
  [[nodiscard]] T as() const {
    return static_cast<T>(ptr_);
  }

  [[nodiscard]] explicit operator bool() const {
    return ptr_ != nullptr;
  }

private:
  CFTypeRef ptr_ = nullptr;
};

/** RAII wrapper for `io_object_t` and derived types. */
struct IoObject {
  IoObject() = default;

  explicit IoObject(io_object_t obj) : obj_{obj} {}

  IoObject(const IoObject&) = delete;
  IoObject& operator=(const IoObject&) = delete;

  IoObject(IoObject&& other) noexcept : obj_{other.obj_} {
    other.obj_ = 0;
  }

  IoObject& operator=(IoObject&& other) noexcept {
    if (this != &other) {
      reset();
      obj_ = other.obj_;
      other.obj_ = 0;
    }
    return *this;
  }

  ~IoObject() {
    reset();
  }

  void reset(io_object_t new_obj = 0) {
    if (obj_ != 0) {
      IOObjectRelease(obj_);
    }
    obj_ = new_obj;
  }

  [[nodiscard]] io_object_t get() const {
    return obj_;
  }

  [[nodiscard]] explicit operator bool() const {
    return obj_ != 0;
  }

private:
  io_object_t obj_ = 0;
};

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
  const auto logical_cores = *thes::safe_cast<std::size_t>(read_sysctl<int>("hw.logicalcpu"));
  if (logical_cores == 0) {
    return {};
  }

  constexpr char dt_plane_name[] = "IODeviceTree";
  constexpr char cpu_plane_path[] = "IODeviceTree:/cpus";

  IoObject cpus_root{IORegistryEntryFromPath(kIOMainPortDefault, cpu_plane_path)};
  if (!cpus_root) {
    return {};
  }

  io_iterator_t raw_iter{};
  if (IORegistryEntryGetChildIterator(cpus_root.get(), dt_plane_name, &raw_iter) != KERN_SUCCESS) {
    return {};
  }

  IoObject cpus_iter{raw_iter};
  std::vector<CpuEntry> entries{};

  while (true) {
    IoObject cpus_child{IOIteratorNext(cpus_iter.get())};
    if (!cpus_child) {
      break;
    }

    CfRef logical_id_ref{IORegistryEntrySearchCFProperty(
      cpus_child.get(), dt_plane_name, CFSTR("logical-cpu-id"), kCFAllocatorDefault, kNilOptions)};
    if (!logical_id_ref) {
      continue;
    }
    if (CFGetTypeID(logical_id_ref.get()) != CFNumberGetTypeID()) {
      continue;
    }

    long long logical_id_ll{};
    const bool ok = CFNumberGetValue(logical_id_ref.as<CFNumberRef>(), kCFNumberLongLongType,
                                     &logical_id_ll) != 0;
    if (!ok || logical_id_ll < 0) {
      continue;
    }

    const auto logical_id = *safe_cast<std::size_t>(logical_id_ll);
    if (logical_id >= logical_cores) {
      continue;
    }

    EfficiencyClass efficiency = EfficiencyClass::any;

    CfRef cluster_ref{IORegistryEntrySearchCFProperty(
      cpus_child.get(), dt_plane_name, CFSTR("cluster-type"), kCFAllocatorDefault, kNilOptions)};
    if (cluster_ref && CFGetTypeID(cluster_ref.get()) == CFDataGetTypeID()) {
      const auto* const data_ref = cluster_ref.as<CFDataRef>();
      const CFIndex length = CFDataGetLength(data_ref);
      if (length >= 1) {
        UInt8 buffer[2] = {0, 0};
        const CFIndex copy_len = length >= 2 ? 2 : length;
        CFDataGetBytes(data_ref, CFRangeMake(0, copy_len), buffer);
        efficiency = cluster_type_to_efficiency_class(buffer[0]);
      }
    }

    entries.push_back({.logical_id = logical_id, .efficiency_class = efficiency});
  }

  if (entries.empty()) {
    return {};
  }

  std::ranges::sort(entries, {}, &CpuEntry::logical_id);

  if (entries.size() != logical_cores) {
    return {};
  }
  for (std::size_t i = 0; i < entries.size(); ++i) {
    if (entries[i].logical_id != i) {
      return {};
    }
  }

  return entries;
}
