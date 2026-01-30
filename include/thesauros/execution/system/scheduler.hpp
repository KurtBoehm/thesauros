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

#include "thesauros/macropolis/platform.hpp"
#include "thesauros/types/primitives.hpp"
#include "thesauros/utility/as-expected.hpp"

namespace thes {
#if THES_LINUX || THES_APPLE
enum struct Scheduler : u8 { fifo = SCHED_FIFO, round_robin = SCHED_RR };

[[nodiscard]] inline std::expected<void, int> set_scheduler(std::thread::native_handle_type thread,
                                                            Scheduler scheduler) {
  const auto int_sched = static_cast<std::underlying_type_t<Scheduler>>(scheduler);

  sched_param param{};
  param.sched_priority = sched_get_priority_max(int_sched);

  const int ret = pthread_setschedparam(thread, int_sched, &param);
  return as_expected(ret);
}
#elif THES_WINDOWS
enum struct Scheduler : u8 { fifo, round_robin };

[[nodiscard]] inline std::expected<void, WINBOOL>
set_scheduler(std::thread::native_handle_type thread, Scheduler /*scheduler*/) {
  const WINBOOL ret = SetThreadPriority(pthread_gethandle(handle), THREAD_PRIORITY_HIGHEST);
  if (ret == 0) {
    return std::unexpected(ret);
  }
  return {};
}
#endif

[[nodiscard]] inline auto set_scheduler(std::thread& thread, Scheduler scheduler) {
  return set_scheduler(thread.native_handle(), scheduler);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_SYSTEM_SCHEDULER_HPP
