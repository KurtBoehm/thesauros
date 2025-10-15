// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_EXECUTION_SYSTEM_SCHEDULER_HPP
#define INCLUDE_THESAUROS_EXECUTION_SYSTEM_SCHEDULER_HPP

#include <expected>
#include <thread>
#include <type_traits>

#include <pthread.h>
#include <sched.h>

#include "thesauros/types/primitives.hpp"
#include "thesauros/utility/as-expected.hpp"

namespace thes {
enum struct Scheduler : u8 { FIFO = SCHED_FIFO, ROUND_ROBIN = SCHED_RR };

inline std::expected<void, int> set_scheduler(std::thread& thread, const Scheduler scheduler) {
  const int int_sched = static_cast<std::underlying_type_t<Scheduler>>(scheduler);
  sched_param param{};
  param.sched_priority = sched_get_priority_max(int_sched);
  const auto ret = pthread_setschedparam(thread.native_handle(), int_sched, &param);
  return as_expected(ret);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_SYSTEM_SCHEDULER_HPP
