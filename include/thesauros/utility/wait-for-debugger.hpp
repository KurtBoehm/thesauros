// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_WAIT_FOR_DEBUGGER_HPP
#define INCLUDE_THESAUROS_UTILITY_WAIT_FOR_DEBUGGER_HPP

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>

#include "thesauros/macropolis/platform.hpp"

#if THES_WINDOWS
#include <windows.h>
#else
#include <csignal>
#include <unistd.h>
#endif

namespace thes {
namespace detail::debug_wait {
inline std::optional<std::string_view> get_env(const char* name) {
  if (char* raw = std::getenv(name)) { // NOLINT
    return std::string_view{raw};
  }
  return std::nullopt;
}

inline std::string_view trim(std::string_view sv) {
  constexpr std::string_view spaces{" \f\n\r\t\v"};

  const auto first = sv.find_first_not_of(spaces);
  if (first == std::string_view::npos) {
    return {};
  }

  const auto last = sv.find_last_not_of(spaces);
  return sv.substr(first, last - first + 1);
}

inline std::string to_lower(std::string_view sv) {
  std::string result{};
  result.reserve(sv.size());

  std::ranges::transform(sv, std::back_inserter(result),
                         [](char c) { return char(std::tolower(c)); });
  return result;
}

inline bool env_flag_enabled(const char* name) {
  auto env = get_env(name);
  if (!env) {
    return false;
  }

  auto trimmed = trim(*env);
  if (trimmed.empty()) {
    return false;
  }

  auto lower = to_lower(trimmed);
  return lower == "1" || lower == "true" || lower == "on";
}

#if !THES_WINDOWS
inline std::atomic_bool& go_flag() {
  static std::atomic_bool flag = false;
  return flag;
}

inline void signal_handler(int /*signo*/) noexcept {
  std::atomic_bool& go_flag = detail::debug_wait::go_flag();
  go_flag.store(true, std::memory_order_release);
  go_flag.notify_one();
}
#endif
} // namespace detail::debug_wait

/**
 * Waits for a debugger to attach at a checkpoint if the environment variable `WAIT_FOR_DEBUGGER`
 * is 1/true/on (case-insensitive). `wait_cb(pid)` is called when the process starts to wait,
 * `done_cb(pid)` when the process is done waiting.
 *
 * On POSIX systems, waiting ends once `SIGUSR1` is received; on Windows, there is no equivalent
 * signal, so the process instead polls `IsDebuggerPresent` until a debugger attaches.
 */
inline void wait_for_debugger(std::invocable<int> auto wait_cb, std::invocable<int> auto done_cb) {
  if (!detail::debug_wait::env_flag_enabled("WAIT_FOR_DEBUGGER")) {
    return;
  }

#if THES_WINDOWS
  const int pid = static_cast<int>(::GetCurrentProcessId());

  wait_cb(pid);

  while (!::IsDebuggerPresent()) {
    ::Sleep(10);
  }

  done_cb(pid);
#else
  std::atomic_bool& go_flag = detail::debug_wait::go_flag();

  go_flag.store(false, std::memory_order_relaxed);

  (void)std::signal(SIGUSR1, detail::debug_wait::signal_handler);

  const int pid = ::getpid();

  wait_cb(pid);

  go_flag.wait(false, std::memory_order_acquire);

  done_cb(pid);
#endif
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_WAIT_FOR_DEBUGGER_HPP
