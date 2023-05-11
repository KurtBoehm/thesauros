#ifndef INCLUDE_THESAUROS_EXECUTION_SYSTEM_SCHEDULER_HPP
#define INCLUDE_THESAUROS_EXECUTION_SYSTEM_SCHEDULER_HPP

#include <thread>
#include <type_traits>

#include <sched.h>

#include "tl/expected.hpp"

#include "thesauros/utility/as-expected.hpp"

namespace thes {
enum struct Scheduler : int { FIFO = SCHED_FIFO, ROUND_ROBIN = SCHED_RR };

[[nodiscard]] inline tl::expected<void, int> set_scheduler(std::thread& thread,
                                                           const Scheduler scheduler) {
  const int int_sched{static_cast<std::underlying_type_t<Scheduler>>(scheduler)};
  sched_param param{sched_get_priority_max(int_sched)};
  const auto ret = pthread_setschedparam(thread.native_handle(), int_sched, &param);
  return as_expected(ret);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_SYSTEM_SCHEDULER_HPP
