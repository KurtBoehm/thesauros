// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_RANGES_REVERSED_HPP
#define INCLUDE_THESAUROS_RANGES_REVERSED_HPP

#include <type_traits>
#include <utility>

namespace thes {
template<typename TRange>
struct ReverseRange {
  using Range = std::decay_t<TRange>;
  using value_type = Range::value_type;
  using const_iterator = Range::const_reverse_iterator;

  explicit constexpr ReverseRange(TRange&& container)
      : container_(std::forward<TRange>(container)) {}

  constexpr const_iterator begin() const {
    return container_.rbegin();
  }

  constexpr const_iterator end() const {
    return container_.rend();
  }

  constexpr bool contains(const value_type& value) const {
    return container_.contains(value);
  }

private:
  TRange container_;
};

template<typename TRange>
inline constexpr auto reversed(TRange&& container) {
  return ReverseRange<TRange>(std::forward<TRange>(container));
}
} // namespace thes

#endif // INCLUDE_THESAUROS_RANGES_REVERSED_HPP
