// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_RANGES_ENUMERATE_HPP
#define INCLUDE_THESAUROS_RANGES_ENUMERATE_HPP

#include <cstddef>
#include <utility>

#include "thesauros/iterator/facade.hpp"
#include "thesauros/iterator/state-facade.hpp"
#include "thesauros/math/integer-cast.hpp"

namespace thes::ranges {
template<typename TSize, typename TIter>
struct EnumerateRange {
  using Value = std::pair<TSize, decltype(*std::declval<TIter>())>;

  struct const_iterator : public StateIteratorFacade<iter::ValueTypes<Value, std::ptrdiff_t>> {
    friend StateIteratorFacade<iter::ValueTypes<Value, std::ptrdiff_t>>;

    constexpr const_iterator() = default;
    explicit constexpr const_iterator(TIter begin, TIter it) : begin_(begin), it_(std::move(it)) {}

  private:
    constexpr Value value() const {
      return Value{*safe_cast<TSize>(it_ - begin_), *it_};
    }

    constexpr auto& state(this auto& self) {
      return self.it_;
    }

    TIter begin_{};
    TIter it_{};
  };

  constexpr EnumerateRange(TIter begin, TIter end)
      : begin_(std::move(begin)), end_(std::move(end)) {}

  constexpr const_iterator begin() const {
    return const_iterator(begin_, begin_);
  }
  constexpr const_iterator end() const {
    return const_iterator(begin_, end_);
  }

private:
  TIter begin_;
  TIter end_;
};
} // namespace thes::ranges

namespace thes::views {
/** Pairs every element of `container` with its zero-based index, of type `TSize`. */
template<typename TSize, typename TRange>
constexpr auto enumerate(TRange&& container) {
  using Iter = decltype(container.begin());
  return ranges::EnumerateRange<TSize, Iter>{container.begin(), container.end()};
}
/** Pairs every element of `[begin, end)` with its zero-based index, of type `TSize`. */
template<typename TSize, typename TIter>
constexpr auto enumerate(TIter begin, TIter end) {
  return ranges::EnumerateRange<TSize, TIter>{std::move(begin), std::move(end)};
}
} // namespace thes::views

#endif // INCLUDE_THESAUROS_RANGES_ENUMERATE_HPP
