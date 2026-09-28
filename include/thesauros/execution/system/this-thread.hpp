// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_EXECUTION_SYSTEM_THIS_THREAD_HPP
#define INCLUDE_THESAUROS_EXECUTION_SYSTEM_THIS_THREAD_HPP

#include <concepts>
#include <thread>

#include <pthread.h>

namespace thes {
/**
 * The native handle of the calling thread, of the type returned by `std::thread::native_handle`.
 */
inline std::thread::native_handle_type this_thread_native_handle() {
  static_assert(std::same_as<std::thread::native_handle_type, pthread_t>,
                "Only pthread-based standard-library threads are supported.");
  return pthread_self();
}
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_SYSTEM_THIS_THREAD_HPP
