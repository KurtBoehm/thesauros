#ifndef INCLUDE_THESAUROS_ALGORITHMS_EXECUTION_TRANSFORM_INCLUSIVE_SCAN_HPP
#define INCLUDE_THESAUROS_ALGORITHMS_EXECUTION_TRANSFORM_INCLUSIVE_SCAN_HPP

#include <cstddef>
#include <iterator>
#include <latch>
#include <numeric>
#include <type_traits>
#include <utility>

#include "thesauros/containers/array/fixed.hpp"
#include "thesauros/utility/safe-cast.hpp"

namespace thes {
template<typename T, typename TExecutionPolicy, typename TForwardIt1, typename TForwardIt2,
         typename TBinaryOperation, typename TUnaryOperation>
inline void transform_inclusive_scan(TExecutionPolicy&& policy, TForwardIt1 first, TForwardIt1 last,
                                     TForwardIt2 d_first, TBinaryOperation binary_op,
                                     TUnaryOperation unary_op, T neutral) {
  const auto raw_size = std::distance(first, last);
  const auto size = [raw_size] {
    using ExPo = std::decay_t<TExecutionPolicy>;
    if constexpr (requires { typename ExPo::Size; }) {
      return *safe_cast<typename ExPo::Size>(raw_size);
    } else {
      using Size = std::make_unsigned_t<std::decay_t<decltype(raw_size)>>;
      return *safe_cast<Size>(raw_size);
    }
  }();

  std::latch barrier{*safe_cast<std::ptrdiff_t>(policy.size())};
  FixedArrayDefault<T> offsets(policy.size());
  std::forward<TExecutionPolicy>(policy).execute_segmented(
    size, [=, &barrier, &offsets](std::size_t thread_idx, auto begin, auto end) {
      auto thread_first = first;
      std::advance(thread_first, begin);

      auto thread_last = first;
      std::advance(thread_last, end);

      const T part = std::transform_reduce(thread_first, thread_last, neutral, binary_op, unary_op);
      offsets[thread_idx] = part;
      barrier.arrive_and_wait();

      const T offset =
        std::reduce(offsets.begin(), offsets.begin() + thread_idx, neutral, binary_op);
      auto thread_d_first = d_first;
      std::advance(thread_d_first, begin);
      std::transform_inclusive_scan(thread_first, thread_last, thread_d_first, binary_op, unary_op,
                                    offset);
    });
}
} // namespace thes

#endif // INCLUDE_THESAUROS_ALGORITHMS_EXECUTION_TRANSFORM_INCLUSIVE_SCAN_HPP
