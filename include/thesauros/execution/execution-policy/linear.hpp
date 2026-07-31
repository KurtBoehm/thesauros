// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_EXECUTION_EXECUTION_POLICY_LINEAR_HPP
#define INCLUDE_THESAUROS_EXECUTION_EXECUTION_POLICY_LINEAR_HPP

#include <cstddef>

#include "thesauros/utility/index-segmentation.hpp"

namespace thes {
template<typename E>
struct LinearExecutionPolicy {
  using Executor = E;

  explicit LinearExecutionPolicy(const E& executor) : executor_(executor) {}

  template<typename S, typename F>
  void execute_segmented(S size, F&& f) const { // NOLINT(*-missing-std-forward)
    UniformIndexSegmenter segmenter(size, executor_.thread_num());
    executor_.execute([&f, &segmenter](std::size_t thread_idx) {
      const auto begin = segmenter.segment_start(thread_idx);
      const auto end = segmenter.segment_end(thread_idx);
      f(thread_idx, begin, end);
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
