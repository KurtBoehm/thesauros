// cpuinfo_iokit.cpp
// clang++ -std=c++23 -Wall -framework IOKit -framework CoreFoundation cpuinfo_iokit.cpp -o
// cpuinfo_iokit

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <print>
#include <string>
#include <string_view>

using std::string;
using std::string_view;

/** Read a little‑endian integer from at most 8 bytes. */
static std::uint64_t read_le_integer(const UInt8* bytes, CFIndex len) {
  std::uint64_t v{0};
  // Little‑endian: least‑significant byte first.
  for (CFIndex i{len - 1}; i >= 0 && i < len && i < 8; --i) {
    v = (v << 8U) | bytes[i];
    if (i == 0) {
      break;
    }
  }
  return v;
}

/** Convert a `CFStringRef` to `std::string`. */
static string cfstring_to_string(CFStringRef str) {
  if (str == nullptr) {
    return "<null>";
  }
  char buf[256]{};
  if (CFStringGetCString(str, buf, sizeof(buf), kCFStringEncodingUTF8) == 0) {
    return "<unprintable CFString>";
  }
  return string{buf};
}

/** Convert a `CFDataRef` treated as C‑string bytes to `std::string`. */
static string cfdata_to_cstring(CFDataRef data) {
  if (data == nullptr) {
    return "<null>";
  }
  const auto* bytes = CFDataGetBytePtr(data);
  auto len = CFDataGetLength(data);
  if (len == 0) {
    return "<empty>";
  }

  char buf[256];
  auto max_copy = static_cast<CFIndex>(sizeof(buf) - 1);
  CFIndex copy_len = len < max_copy ? len : max_copy;
  std::memcpy(buf, bytes, static_cast<std::size_t>(copy_len));
  buf[copy_len] = '\0';
  return string{buf};
}

/** Print the “compatible” string list. */
static void print_compatible(CFDataRef data) {
  if (data == nullptr) {
    std::println("  compatible: <none>");
    return;
  }
  const auto* bytes = CFDataGetBytePtr(data);
  auto len = CFDataGetLength(data);
  std::print("  compatible:");
  CFIndex i{0};
  while (i < len) {
    const char* s = reinterpret_cast<const char*>(&bytes[i]);
    std::size_t slen = std::strlen(s);
    if (slen == 0) {
      break;
    }
    std::print(" {}", s);
    i += static_cast<CFIndex>(slen + 1);
  }
  std::print("\n");
}

/** Print values that are either `CFNumber` or `CFData` holding an integer. */
static void print_integer_like(const char* label, CFTypeRef val) {
  if (val == nullptr) {
    return;
  }
  auto tid = CFGetTypeID(val);

  if (tid == CFNumberGetTypeID()) {
    long long n{0};
    CFNumberGetValue(static_cast<CFNumberRef>(val), kCFNumberLongLongType, &n);
    std::println("  {}: {} (0x{:x}) [CFNumber]", label, n, n);
  } else if (tid == CFDataGetTypeID()) {
    auto data = static_cast<CFDataRef>(val);
    const auto* bytes = CFDataGetBytePtr(data);
    auto len = CFDataGetLength(data);
    if (len == 0) {
      return;
    }
    auto v = read_le_integer(bytes, len);
    std::println("  {}: {} (0x{:x}) [CFData len={}]", label, static_cast<unsigned long long>(v),
                 static_cast<unsigned long long>(v), static_cast<long>(len));
  }
}

/** Print values that are either `CFNumber` or `CFData` holding a frequency in Hz. */
static void print_frequency_like(const char* label, CFTypeRef val) {
  if (val == nullptr) {
    return;
  }
  auto tid = CFGetTypeID(val);

  std::uint64_t hz{0};
  if (tid == CFNumberGetTypeID()) {
    CFNumberGetValue(static_cast<CFNumberRef>(val), kCFNumberSInt64Type, &hz);
  } else if (tid == CFDataGetTypeID()) {
    auto data = static_cast<CFDataRef>(val);
    const auto* bytes = CFDataGetBytePtr(data);
    auto len = CFDataGetLength(data);
    if (len == 0) {
      return;
    }
    hz = read_le_integer(bytes, len);
  } else {
    return;
  }

  double ghz = static_cast<double>(hz) / 1e9;
  std::println("  {}: {} Hz ({:.3f} GHz)", label, static_cast<unsigned long long>(hz), ghz);
}

/** Print the cluster type from `CFString` or `CFData`. */
static void print_cluster_type(CFTypeRef val) {
  if (val == nullptr) {
    std::println("  cluster-type: <none>");
    return;
  }
  auto tid = CFGetTypeID(val);
  if (tid != CFDataGetTypeID() && tid != CFStringGetTypeID()) {
    std::println("  cluster-type: <non-string type>");
    return;
  }

  char buf[8]{0};

  if (tid == CFStringGetTypeID()) {
    CFStringGetCString(static_cast<CFStringRef>(val), buf, sizeof(buf), kCFStringEncodingUTF8);
  } else {
    auto data = static_cast<CFDataRef>(val);
    const auto* bytes = CFDataGetBytePtr(data);
    auto len = CFDataGetLength(data);
    if (len == 0) {
      std::println("  cluster-type: <empty>");
      return;
    }
    auto max_copy = static_cast<CFIndex>(sizeof(buf) - 1);
    CFIndex copy_len = len < max_copy ? len : max_copy;
    std::memcpy(buf, bytes, static_cast<std::size_t>(copy_len));
    buf[copy_len] = '\0';
  }

  const char* kind = "unknown";
  if (buf[0] == 'P' || buf[0] == 'p') {
    kind = "performance";
  } else if (buf[0] == 'E' || buf[0] == 'e') {
    kind = "efficiency";
  } else if (buf[0] == 'M' || buf[0] == 'm') {
    kind = "mixed / mid";
  }

  std::println("  cluster-type: \"{}\" ({} core)", buf, kind);
}

/** Print a property that indicates presence or boolean. */
static void print_presence_boolean(const char* label, CFTypeRef val) {
  if (val == nullptr) {
    std::println("  {}: false", label);
    return;
  }
  auto tid = CFGetTypeID(val);
  if (tid == CFBooleanGetTypeID()) {
    bool b = CFBooleanGetValue(static_cast<CFBooleanRef>(val)) != 0;
    std::println("  {}: {}", label, b ? "true" : "false");
  } else if (tid == CFDataGetTypeID()) {
    auto len = CFDataGetLength(static_cast<CFDataRef>(val));
    std::println("  {}: true (CFData len={})", label, static_cast<long>(len));
  } else {
    std::println("  {}: <non-boolean type>", label);
  }
}

/** Inspect and print a single CPU node. */
static void inspect_cpu_node(io_registry_entry_t cpu) {
  CFMutableDictionaryRef props = nullptr;
  if (IORegistryEntryCreateCFProperties(cpu, &props, kCFAllocatorDefault, 0) != KERN_SUCCESS ||
      props == nullptr) {
    return;
  }

  std::println("CPU-like node:");

  io_string_t path;
  if (IORegistryEntryGetPath(cpu, kIOServicePlane, path) == KERN_SUCCESS) {
    std::println("  path: {}", path);
  }

  // name
  {
    auto name = CFDictionaryGetValue(props, CFSTR("name"));
    std::print("  name: ");
    if (name != nullptr) {
      auto tid = CFGetTypeID(name);
      if (tid == CFDataGetTypeID()) {
        std::println("{}", cfdata_to_cstring(static_cast<CFDataRef>(name)));
      } else if (tid == CFStringGetTypeID()) {
        std::println("{}", cfstring_to_string(static_cast<CFStringRef>(name)));
      } else {
        std::println("<type {}>", static_cast<unsigned long>(tid));
      }
    } else {
      std::println("<missing>");
    }
  }

  // device_type
  {
    auto dev_type = CFDictionaryGetValue(props, CFSTR("device_type"));
    std::print("  device_type: ");
    if (dev_type != nullptr) {
      auto tid = CFGetTypeID(dev_type);
      if (tid == CFDataGetTypeID()) {
        std::println("{}", cfdata_to_cstring(static_cast<CFDataRef>(dev_type)));
      } else if (tid == CFStringGetTypeID()) {
        std::println("{}", cfstring_to_string(static_cast<CFStringRef>(dev_type)));
      } else {
        std::println("<type {}>", static_cast<unsigned long>(tid));
      }
    } else {
      std::println("<missing>");
    }
  }

  // compatible
  {
    auto compat = CFDictionaryGetValue(props, CFSTR("compatible"));
    if (compat != nullptr && CFGetTypeID(compat) == CFDataGetTypeID()) {
      print_compatible(static_cast<CFDataRef>(compat));
    } else {
      std::println("  compatible: <none or non-data>");
    }
  }

  // Integer-ish identifiers.
  print_integer_like("logical-cluster-id",
                     CFDictionaryGetValue(props, CFSTR("logical-cluster-id")));
  print_integer_like("logical-cpu-id", CFDictionaryGetValue(props, CFSTR("logical-cpu-id")));
  print_integer_like("cluster-id", CFDictionaryGetValue(props, CFSTR("cluster-id")));
  print_integer_like("cluster-core-id", CFDictionaryGetValue(props, CFSTR("cluster-core-id")));
  print_integer_like("die-id", CFDictionaryGetValue(props, CFSTR("die-id")));
  print_integer_like("die-cluster-id", CFDictionaryGetValue(props, CFSTR("die-cluster-id")));
  print_integer_like("cpu-id", CFDictionaryGetValue(props, CFSTR("cpu-id")));
  print_integer_like("l2-cache-id", CFDictionaryGetValue(props, CFSTR("l2-cache-id")));
  print_integer_like("l2-cache-size", CFDictionaryGetValue(props, CFSTR("l2-cache-size")));
  print_integer_like("reg", CFDictionaryGetValue(props, CFSTR("reg")));

  // Cluster type.
  print_cluster_type(CFDictionaryGetValue(props, CFSTR("cluster-type")));

  // Frequencies.
  print_frequency_like("clock-frequency", CFDictionaryGetValue(props, CFSTR("clock-frequency")));
  print_frequency_like("timebase-frequency",
                       CFDictionaryGetValue(props, CFSTR("timebase-frequency")));
  print_frequency_like("bus-frequency", CFDictionaryGetValue(props, CFSTR("bus-frequency")));
  print_frequency_like("memory-frequency", CFDictionaryGetValue(props, CFSTR("memory-frequency")));
  print_frequency_like("peripheral-frequency",
                       CFDictionaryGetValue(props, CFSTR("peripheral-frequency")));

  // Booleans/presence.
  print_presence_boolean("fixed-frequency", CFDictionaryGetValue(props, CFSTR("fixed-frequency")));
  print_presence_boolean("no-aic-ipi-required",
                         CFDictionaryGetValue(props, CFSTR("no-aic-ipi-required")));

  CFRelease(props);
  std::print("\n");
}

/** Entry point. */
int main() {
  kern_return_t kr{};
  io_iterator_t iter = IO_OBJECT_NULL;

  io_service_t armpe =
    IOServiceGetMatchingService(kIOMainPortDefault, IOServiceMatching("AppleARMPE"));
  if (armpe == 0) {
    std::println(stderr, "AppleARMPE not found");
    return 1;
  }

  kr = IORegistryEntryGetChildIterator(armpe, kIOServicePlane, &iter);
  IOObjectRelease(armpe);
  if (kr != KERN_SUCCESS) {
    std::println(stderr, "IORegistryEntryGetChildIterator(AppleARMPE) failed: 0x{:x}",
                 static_cast<unsigned int>(kr));
    return 1;
  }

  std::println("Scanning AppleARMPE children for CPU nodes:\n");

  io_registry_entry_t child{};
  while ((child = IOIteratorNext(iter)) != IO_OBJECT_NULL) {
    CFTypeRef name_prop =
      IORegistryEntryCreateCFProperty(child, CFSTR("name"), kCFAllocatorDefault, 0);
    bool is_cpu = false;
    if (name_prop != nullptr && CFGetTypeID(name_prop) == CFDataGetTypeID()) {
      auto data = static_cast<CFDataRef>(name_prop);
      const auto* bytes = CFDataGetBytePtr(data);
      auto len = CFDataGetLength(data);
      if (len >= 3 && std::memcmp(bytes, "cpu", 3) == 0) {
        is_cpu = true;
      }
    }
    if (name_prop != nullptr) {
      CFRelease(name_prop);
    }

    if (is_cpu) {
      inspect_cpu_node(child);
    }

    IOObjectRelease(child);
  }
  IOObjectRelease(iter);

  return EXIT_SUCCESS;
}
