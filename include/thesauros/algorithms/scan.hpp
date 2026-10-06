// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_ALGORITHMS_SCAN_HPP
#define INCLUDE_THESAUROS_ALGORITHMS_SCAN_HPP

#include <algorithm>
#include <atomic>
#include <concepts>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

#include "thesauros/execution/execution-policy/core.hpp"
#include "thesauros/execution/system/spin.hpp"
#include "thesauros/math/arithmetic.hpp"
#include "thesauros/math/integer-cast.hpp"
#include "thesauros/ranges/indices.hpp"
#include "thesauros/types/empty.hpp"
#include "thesauros/types/primitives.hpp"

namespace thes::ranges {
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
  template<std::ranges::input_range R, std::weakly_incrementable O, typename Op, typename T>
  constexpr std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, O>
  operator()(R&& r, O out, Op op, T init) const {
    return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(out), std::move(op),
                   std::move(init));
  }

  /** Sequential inclusive scan without an initial value. */
  template<std::ranges::input_range R, std::weakly_incrementable O, typename Op = std::plus<>>
  constexpr std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, O>
  operator()(R&& r, O out, Op op = {}) const {
    return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(out), std::move(op));
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
  /** Per-chunk state for the look-back; one cache line per chunk to avoid false sharing. */
  template<typename T>
  struct alignas(128) ChunkState {
    enum : u8 { empty, aggregate_ready, prefix_ready };

    std::atomic<u8> flag{empty};
    // Written once, before `flag = aggregate_ready`.
    std::optional<T> aggregate{};
    // Written once, before `flag = prefix_ready`.
    std::optional<T> prefix{};
  };

  /** Below this many bytes of `T`, the sequential scan wins. */
  static constexpr std::size_t min_parallel_bytes = 1UZ << 17; // 128 KiB
  /** Chunk size bounds in bytes of `T`. */
  static constexpr std::size_t min_chunk_bytes = 1UZ << 14; // 16 KiB
  static constexpr std::size_t max_chunk_bytes = 1UZ << 16; // 64 KiB

  /**
   * Single-pass parallel inclusive scan with decoupled look-back, following
   *   D. Merrill and M. Garland, “Single-pass Parallel Prefix Scan with Decoupled Look-back”,
   *   NVIDIA Technical Report NVR-2016-002, 2016.
   *   https://research.nvidia.com/publication/2016-03_single-pass-parallel-prefix-scan-decoupled-look-back
   *
   * Uses the basic algorithm (sequential look-back; flag published after the value with release
   * semantics) and the paper’s atomic-counter partition indices, claimed repeatedly by one call per
   * thread. Additions: `init` is folded into the first chunk’s prefix; chunks are sized to stay in
   * cache between reduction and scan, and computed inputs are buffered to be evaluated only once.
   *
   * As in the paper, the order in which `op` is applied depends on scheduling, so results for
   * pseudo-associative operators (e.g., floating-point addition) may differ between runs.
   */
  template<typename T, typename P, typename I, typename O, typename Op, typename Init = Empty>
  std::ranges::in_out_result<I, O> parallel(P& policy, I first, std::iter_difference_t<I> n, O out,
                                            Op& op, Init init = {}) const {
    using V = std::iter_value_t<I>;
    using D = std::iter_difference_t<I>;
    using State = ChunkState<T>;
    static constexpr bool has_init = !std::same_as<Init, Empty>;

    // Dereferencing produces a new value (e.g., `views::transform`): evaluate each element only
    // once.
    static constexpr bool buffer_input = !std::is_reference_v<std::iter_reference_t<I>>;
    using Buffer = std::conditional_t<buffer_input, std::vector<V>, Empty>;

    const std::size_t thread_num = policy.thread_num();
    const std::size_t nz = *safe_cast<std::size_t>(n);

    // Not worth parallelizing (this also covers n == 0).
    if (thread_num <= 1 || nz < min_parallel_bytes / sizeof(T)) {
      if constexpr (has_init) {
        return (*this)(first, first + n, out, op, std::move(init));
      } else {
        return (*this)(first, first + n, out, op);
      }
    }

    // Aim for at least 4 chunks per thread, within the size bounds.
    const std::size_t min_grain = std::max(1UZ, min_chunk_bytes / sizeof(T));
    const std::size_t max_grain = std::max(min_grain, max_chunk_bytes / sizeof(T));
    const std::size_t grain = std::clamp(div_ceil(nz, 4 * thread_num), min_grain, max_grain);
    const D diff_grain = *safe_cast<D>(grain);

    const auto chunk_num = div_ceil(nz, grain);
    auto states = std::make_unique<State[]>(chunk_num);

    struct alignas(128) PaddedCounter {
      std::atomic<std::size_t> value{0};
    };
    PaddedCounter next_chunk{};

    auto process_chunk = [&](std::size_t c, Buffer& buffer) {
      const D begin = *safe_cast<D>(c * grain);
      const D end = std::min<D>(begin + diff_grain, n);
      State& state = states[c];

      // Reduce the chunk; computed elements are buffered so that they are evaluated only once.
      T aggregate = [&] {
        if constexpr (buffer_input) {
          buffer.clear();
          buffer.push_back(first[begin]);
          T acc = buffer.back();
          for (const D i : views::indices(begin + 1, end)) {
            buffer.push_back(first[i]);
            acc = std::invoke(op, std::move(acc), buffer.back());
          }
          return acc;
        } else {
          T acc = first[begin];
          for (const D i : views::indices(begin + 1, end)) {
            acc = std::invoke(op, std::move(acc), first[i]);
          }
          return acc;
        }
      }();

      // Determine the exclusive prefix: publish the aggregate, then look back.
      std::optional<T> exclusive{};
      if (c == 0) {
        if constexpr (has_init) {
          exclusive.emplace(init);
        }
      } else {
        state.aggregate.emplace(aggregate);
        state.flag.store(State::aggregate_ready, std::memory_order_release);

        for (const std::size_t j : std::views::reverse(views::indices(c))) {
          State& pred = states[j];
          u8 flag{};
          while ((flag = pred.flag.load(std::memory_order_acquire)) == State::empty) {
            spin_pause();
          }
          // Predecessors are combined from the left.
          const T& value = (flag == State::prefix_ready) ? *pred.prefix : *pred.aggregate;
          exclusive = exclusive ? T(std::invoke(op, value, std::move(*exclusive))) : T(value);
          if (flag == State::prefix_ready) {
            break;
          }
        }
      }

      // Publish the inclusive prefix before scanning, so that successors are unblocked early.
      if (exclusive) {
        state.prefix.emplace(std::invoke(op, *exclusive, std::move(aggregate)));
      } else {
        state.prefix.emplace(std::move(aggregate));
      }
      state.flag.store(State::prefix_ready, std::memory_order_release);

      // Scan the chunk from the buffer or the (cache-resident) input, starting from its offset.
      auto scan = [&]<typename... Arg>(Arg&&... init_arg) {
        if constexpr (buffer_input) {
          (*this)(std::make_move_iterator(buffer.begin()), std::make_move_iterator(buffer.end()),
                  out + begin, op, std::forward<Arg>(init_arg)...);
        } else {
          (*this)(first + begin, first + end, out + begin, op, std::forward<Arg>(init_arg)...);
        }
      };
      if (exclusive) {
        scan(std::move(*exclusive));
      } else {
        scan();
      }
    };

    // Chunks are claimed through the ticket counter in ascending order, which balances load and
    // keeps the look-back safe.
    policy.execute_threaded([&](std::size_t /*thread_idx*/) noexcept {
      Buffer buffer{};
      if constexpr (buffer_input) {
        // One allocation per thread and call; no reallocation per chunk.
        buffer.reserve(grain);
      }
      auto fetch_chunk = [&] { return next_chunk.value.fetch_add(1, std::memory_order_relaxed); };
      for (std::size_t c = fetch_chunk(); c < chunk_num; c = fetch_chunk()) {
        process_chunk(c, buffer);
      }
    });

    return {first + n, out + n};
  }
};

inline constexpr InclusiveScanFn inclusive_scan{};
} // namespace thes::ranges

#endif // INCLUDE_THESAUROS_ALGORITHMS_SCAN_HPP
