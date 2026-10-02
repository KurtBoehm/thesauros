// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_ALGORITHMS_SCAN_HPP
#define INCLUDE_THESAUROS_ALGORITHMS_SCAN_HPP

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <iterator>
#include <optional>
#include <ranges>
#include <utility>

#include "thesauros/containers/array/fixed.hpp"
#include "thesauros/execution/execution-policy/core.hpp"
#include "thesauros/types/empty.hpp"

namespace thes {
struct InclusiveScanFn {
  //================================================================================================
  // Sequential
  //================================================================================================

  /** Sequential inclusive scan with an initial value. */
  template<std::input_iterator I, std::sentinel_for<I> S, std::weakly_incrementable O, typename Op,
           typename T>
  constexpr std::ranges::in_out_result<I, O> operator()(I first, S last, O out, Op op,
                                                        T init) const {
    for (; first != last; ++first, (void)++out) {
      init = std::invoke(op, std::move(init), *first);
      *out = init;
    }
    return {std::move(first), std::move(out)};
  }

  /** Sequential inclusive scan without an initial value. */
  template<std::input_iterator I, std::sentinel_for<I> S, std::weakly_incrementable O,
           typename Op = std::plus<>>
  constexpr std::ranges::in_out_result<I, O> operator()(I first, S last, O out, Op op = {}) const {
    if (first == last) {
      return {std::move(first), std::move(out)};
    }

    std::iter_value_t<I> acc = *first;
    *out = acc;
    ++first;
    ++out;
    return (*this)(std::move(first), std::move(last), std::move(out), std::move(op),
                   std::move(acc));
  }

  /** Sequential inclusive scan with an initial value. */
  template<std::ranges::input_range R, std::weakly_incrementable O, typename Op = std::plus<>>
  constexpr std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, O>
  operator()(R&& r, O out, Op op = {}) const {
    return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(out), std::move(op));
  }

  /** Sequential inclusive scan without an initial value. */
  template<std::ranges::input_range R, std::weakly_incrementable O, typename Op, typename T>
  constexpr std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, O>
  operator()(R&& r, O out, Op op, T init) const {
    return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(out), std::move(op),
                   std::move(init));
  }

  //================================================================================================
  // Parallel
  //================================================================================================

  /** Parallel inclusive scan with an initial value. */
  template<ExecutionPolicy P, std::random_access_iterator I, std::sized_sentinel_for<I> S,
           std::random_access_iterator O, typename Op, typename T>
  std::ranges::in_out_result<I, O> operator()(P&& policy, I first, S last, O out, Op op,
                                              T init) const {
    return parallel<T>(policy, std::move(first), last - first, std::move(out), op, std::move(init));
  }

  /** Parallel inclusive scan without an initial value. */
  template<ExecutionPolicy P, std::random_access_iterator I, std::sized_sentinel_for<I> S,
           std::random_access_iterator O, typename Op = std::plus<>>
  std::ranges::in_out_result<I, O> operator()(P&& policy, I first, S last, O out,
                                              Op op = {}) const {
    return parallel<std::iter_value_t<I>>(policy, std::move(first), last - first, std::move(out),
                                          op);
  }

  /** Parallel inclusive scan with an initial value. */
  template<ExecutionPolicy P, std::ranges::random_access_range R, std::random_access_iterator O,
           typename Op, typename T>
  requires std::ranges::sized_range<R>
  std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, O>
  operator()(P&& policy, R&& r, O out, Op op, T init) const {
    return parallel<T>(policy, std::ranges::begin(r), std::ranges::distance(r), std::move(out), op,
                       std::move(init));
  }

  /** Parallel inclusive scan without an initial value. */
  template<ExecutionPolicy P, std::ranges::random_access_range R, std::random_access_iterator O,
           typename Op = std::plus<>>
  requires std::ranges::sized_range<R>
  std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, O>
  operator()(P&& policy, R&& r, O out, Op op = {}) const {
    return parallel<std::ranges::range_value_t<R>>(policy, std::ranges::begin(r),
                                                   std::ranges::distance(r), std::move(out), op);
  }

private:
  template<typename T, typename P, typename I, typename O, typename Op, typename Init = Empty>
  std::ranges::in_out_result<I, O> parallel(P& policy, I first, std::iter_difference_t<I> n, O out,
                                            Op& op, Init init = {}) const {
    if (n == 0) {
      return {std::move(first), std::move(out)};
    }

    FixedArray<std::optional<T>> offsets(policy.thread_num());

    // Phase 1: Compute the total for each chunk.
    policy.execute_segmented(
      n,
      [&](std::size_t thread_idx, auto begin, auto end) noexcept {
        assert(end > begin);
        I it = first + begin;
        I last = first + end;

        T acc = *it;
        for (++it; it != last; ++it) {
          acc = std::invoke(op, std::move(acc), *it);
        }
        offsets[thread_idx].emplace(std::move(acc));
      },
      once_per_thread);

    // Phase 2: Perform the actual scan with the offsets from phase 1.
    policy.execute_segmented(
      n,
      [&](std::size_t thread_idx, auto begin, auto end) noexcept {
        if (begin == 0) {
          if constexpr (!std::same_as<Init, Empty>) {
            (*this)(first + begin, first + end, out + begin, op, init);
          } else {
            (*this)(first + begin, first + end, out + begin, op);
          }
        } else {
          auto oit = offsets.begin();
          assert(oit->has_value());
          auto oend = offsets.begin() + thread_idx;
          if constexpr (!std::same_as<Init, Empty>) {
            T offset = init;
            for (; oit != oend; ++oit) {
              offset = std::invoke(op, std::move(offset), **oit);
            }
            (*this)(first + begin, first + end, out + begin, op, std::move(offset));
          } else {
            T offset = **oit;
            for (++oit; oit != oend; ++oit) {
              offset = std::invoke(op, std::move(offset), **oit);
            }
            (*this)(first + begin, first + end, out + begin, op, std::move(offset));
          }
        }
      },
      once_per_thread);

    return {first + n, out + n};
  }
};
} // namespace thes

#endif // INCLUDE_THESAUROS_ALGORITHMS_SCAN_HPP
