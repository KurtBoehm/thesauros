// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_RESOURCES_RESOURCE_USAGE_HPP
#define INCLUDE_THESAUROS_RESOURCES_RESOURCE_USAGE_HPP

#include <sys/resource.h>

#include "thesauros/quantity/quantity.hpp"

namespace thes {
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
  Memory max_memory_;
  Duration user_time_;
  Duration system_time_;

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
};
} // namespace thes

#endif // INCLUDE_THESAUROS_RESOURCES_RESOURCE_USAGE_HPP
