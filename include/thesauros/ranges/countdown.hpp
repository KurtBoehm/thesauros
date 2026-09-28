// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_RANGES_COUNTDOWN_HPP
#define INCLUDE_THESAUROS_RANGES_COUNTDOWN_HPP

#include <concepts>
#include <cstddef>
#include <type_traits>

namespace thes::ranges {
/**
 * A view that counts from `n` down to `1` (inclusive), which yields the most efficient range-based
 * for loop with `n` elements.
 * Clang seems to be smart enough to re-write a `views::indices`-based loop in the same terms if the
 * actual values are not used, but GCC 16 insists on keeping an increasing accumulator, which is
 * slightly less efficient.
 */
template<std::unsigned_integral S = std::size_t>
struct Countdown {
  /** The number of repetitions. */
  S n;

  struct Iterator {
    using value_type = S;
    using difference_type = std::make_signed_t<S>;

    S i;

    S operator*() const {
      return i;
    }
    Iterator& operator++() {
      --i;
      return *this;
    }
    Iterator operator++(int) {
      Iterator copy = *this;
      ++(*this);
      return copy;
    }
    bool operator==(const Iterator& other) const = default;
  };

  [[nodiscard]] Iterator begin() const {
    return {.i = n};
  }
  [[nodiscard]] Iterator end() const {
    return {.i = 0};
  }
};
} // namespace thes::ranges

namespace thes::views {
/**
 * A view that counts down from `n` to `1`, which produces `n` iterations in the most efficient way
 * possible. See the documentation of `Countdown` for more details.
 */
template<std::unsigned_integral S>
[[nodiscard]] inline ranges::Countdown<S> countdown(S n) {
  return ranges::Countdown{.n = n};
}
} // namespace thes::views

#endif // INCLUDE_THESAUROS_RANGES_COUNTDOWN_HPP
