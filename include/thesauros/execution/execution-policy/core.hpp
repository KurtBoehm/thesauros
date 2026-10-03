// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_EXECUTION_EXECUTION_POLICY_CORE_HPP
#define INCLUDE_THESAUROS_EXECUTION_EXECUTION_POLICY_CORE_HPP

#include <concepts>
#include <cstddef>

namespace thes {
/**
 * At most one segment per thread may be executed. Each segment must be non-empty and repeated calls
 * to `execute_segmented` must use the same distribution of indices onto threads. Segments are
 * contiguous, and they are assigned to threads in ascending order; if fewer segments than threads
 * are used, they are assigned to the threads with the lowest indices. This must be the default
 * behavior when no mode is given.
 */
struct OncePerThread {};
inline constexpr OncePerThread once_per_thread{};

template<typename T>
concept ThreadExecutionMode = std::same_as<T, OncePerThread>;

template<typename T>
concept ExecutionPolicy = requires(const T& expo) {
  {
    expo.execute_segmented(
      0UZ, [](std::size_t, std::size_t, std::size_t) noexcept {}, once_per_thread)
  } -> std::same_as<void>;
  {
    expo.execute_segmented(0UZ, [](std::size_t, std::size_t, std::size_t) noexcept {})
  } -> std::same_as<void>;

  {
    expo.execute_threaded([](std::size_t) noexcept {})
  } -> std::same_as<void>;

  { expo.thread_num() } -> std::same_as<std::size_t>;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_EXECUTION_POLICY_CORE_HPP
