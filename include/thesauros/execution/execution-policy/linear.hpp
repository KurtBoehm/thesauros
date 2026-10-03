// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_EXECUTION_EXECUTION_POLICY_LINEAR_HPP
#define INCLUDE_THESAUROS_EXECUTION_EXECUTION_POLICY_LINEAR_HPP

#include <cstddef>

#include "thesauros/concepts/nothrow.hpp"
#include "thesauros/execution/execution-policy/core.hpp"
#include "thesauros/utility/index-segmentation.hpp"

namespace thes {
template<typename E>
struct LinearExecutionPolicy {
  using Executor = E;

  explicit LinearExecutionPolicy(const E& executor) : executor_{executor} {}

  template<typename F>
  requires(NothrowInvocable<F&, std::size_t>)
  void execute_threaded(F&& f) const {
    executor_.execute(std::forward<F>(f));
  }
  template<typename S, typename F, ThreadExecutionMode Mode = OncePerThread>
  requires(NothrowInvocable<F&, std::size_t, S, S>)
  void execute_segmented(S size, F&& f, Mode /*mode*/ = {}) const {
    UniformIndexSegmenter segmenter{
      size,
      std::min(executor_.thread_num(), *thes::safe_cast<std::size_t>(size)),
    };
    executor_.execute([&f, &segmenter, size](std::size_t thread_idx) noexcept {
      if (std::cmp_greater_equal(thread_idx, size)) {
        return;
      }
      f(thread_idx, segmenter.segment_start(thread_idx), segmenter.segment_end(thread_idx));
    });
  }

  [[nodiscard]] std::size_t thread_num() const {
    return executor_.thread_num();
  }

  [[nodiscard]] const Executor& executor() const {
    return executor_;
  }

private:
  const Executor& executor_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_EXECUTION_POLICY_LINEAR_HPP
