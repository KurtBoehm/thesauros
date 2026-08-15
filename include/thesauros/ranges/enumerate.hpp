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
template<typename S, typename It>
struct EnumerateRange {
  using Value = std::pair<S, decltype(*std::declval<It>())>;

  struct ConstIterator : public StateIteratorFacade<iter::ValueTypes<Value, std::ptrdiff_t>> {
    friend StateIteratorFacade<iter::ValueTypes<Value, std::ptrdiff_t>>;

    constexpr ConstIterator() = default;
    explicit constexpr ConstIterator(It begin, It it) : begin_(begin), it_(std::move(it)) {}

  private:
    [[nodiscard]] constexpr Value value() const {
      return Value{*safe_cast<S>(it_ - begin_), *it_};
    }

    constexpr auto& state(this auto& self) {
      return self.it_;
    }

    It begin_{};
    It it_{};
  };
  using const_iterator = ConstIterator;

  constexpr EnumerateRange(It begin, It end) : begin_(std::move(begin)), end_(std::move(end)) {}

  [[nodiscard]] constexpr ConstIterator begin() const {
    return ConstIterator(begin_, begin_);
  }
  [[nodiscard]] constexpr ConstIterator end() const {
    return ConstIterator(begin_, end_);
  }

private:
  It begin_;
  It end_;
};
} // namespace thes::ranges

namespace thes::views {
/** Pairs every element of `container` with its zero-based index, of type `S`. */
template<typename S, typename R>
constexpr auto enumerate(R&& container) {
  using Iter = decltype(container.begin());
  return ranges::EnumerateRange<S, Iter>{container.begin(), container.end()};
}
/** Pairs every element of `[begin, end)` with its zero-based index, of type `S`. */
template<typename S, typename It>
constexpr auto enumerate(It begin, It end) {
  return ranges::EnumerateRange<S, It>{std::move(begin), std::move(end)};
}
} // namespace thes::views

#endif // INCLUDE_THESAUROS_RANGES_ENUMERATE_HPP
