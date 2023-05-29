#ifndef INCLUDE_THESAUROS_EXECUTION_EXECUTION_POLICY_LINEAR_HPP
#define INCLUDE_THESAUROS_EXECUTION_EXECUTION_POLICY_LINEAR_HPP

#include <cstddef>

#include "thesauros/utility/index-segmentation.hpp"

namespace thes {
template<typename TExecutor>
struct LinearExecutionPolicy {
  using Executor = TExecutor;

  explicit LinearExecutionPolicy(const TExecutor& executor) : executor_(executor) {}

  template<typename TSize, typename TOp>
  void execute_segmented(TSize size, TOp&& op) const {
    UniformIndexSegmenter<TSize> segmenter(size, executor_.size());
    executor_.execute([&op, &segmenter](TSize thread_idx) {
      const auto begin = segmenter.segment_start(thread_idx);
      const auto end = segmenter.segment_end(thread_idx);
      op(thread_idx, begin, end);
    });
  }
  [[nodiscard]] std::size_t size() const {
    return executor_.size();
  }

  [[nodiscard]] const Executor& executor() const {
    return executor_;
  }

private:
  const Executor& executor_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_EXECUTION_POLICY_LINEAR_HPP
