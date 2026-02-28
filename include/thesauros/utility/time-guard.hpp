// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_TIME_GUARD_HPP
#define INCLUDE_THESAUROS_UTILITY_TIME_GUARD_HPP

#include <chrono>
#include <type_traits>

namespace thes {
/** RAII class to add the duration of a scope to a `std::chrono::duration`. */
template<typename TDuration, typename TClock = std::chrono::steady_clock>
requires(!std::is_const_v<TDuration>) // std::chrono::is_clock_v<TClock>
struct TimeGuard {
  using Clock = TClock;
  using TimePoint = Clock::time_point;
  using Duration = TDuration;

  explicit TimeGuard(Duration& dur) : dur_(dur) {}
  TimeGuard(const TimeGuard&) = delete;
  TimeGuard& operator=(const TimeGuard&) = delete;
  TimeGuard(TimeGuard&&) = delete;
  TimeGuard& operator=(TimeGuard&&) = delete;

  ~TimeGuard() {
    dur_ += std::chrono::duration_cast<Duration>(Clock::now() - init_);
  }

private:
  TimePoint init_ = Clock::now();
  Duration& dur_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_TIME_GUARD_HPP
