// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_RANGES_ZIP_HPP
#define INCLUDE_THESAUROS_RANGES_ZIP_HPP

#include <cassert>
#include <cstddef>
#include <type_traits>

#include "thesauros/iterator/facade.hpp"
#include "thesauros/static-ranges/definitions/get-at.hpp"
#include "thesauros/static-ranges/piping.hpp" // IWYU pragma: keep
#include "thesauros/static-ranges/sinks/for-each.hpp"
#include "thesauros/static-ranges/sinks/to-tuple.hpp"
#include "thesauros/static-ranges/sinks/unique-value.hpp"
#include "thesauros/static-ranges/views/transform.hpp"
#include "thesauros/types/tuple.hpp"

namespace thes {
template<typename... TRanges>
struct ZipRange {
  using Value = Tuple<decltype(*std::declval<typename std::decay_t<TRanges>::const_iterator>())...>;
  using Iterators = Tuple<typename std::decay_t<TRanges>::const_iterator...>;

  struct ConstIterator : public IteratorFacade<iter::ValueTypes<Value, std::ptrdiff_t>> {
    friend IteratorFacade<iter::ValueTypes<Value, std::ptrdiff_t>>;

    constexpr ConstIterator() = default;
    explicit constexpr ConstIterator(Iterators&& iterators) : its_(std::move(iterators)) {}

  private:
    constexpr Value deref() const {
      return its_ | star::transform([](const auto& it) -> decltype(auto) { return *it; }) |
             star::to_tuple;
    }
    constexpr void incr() {
      its_ | star::for_each([](auto& it) { ++it; });
    }
    constexpr bool eq(const ConstIterator& other) const {
      assert(star::transform([](auto i1, auto i2) { return i1 == i2; }, its_, other.its_) |
             star::has_unique_value);
      return star::get_at<0>(its_) == star::get_at<0>(other.its_);
    }

    Iterators its_{};
  };

  using const_iterator = ConstIterator;

  explicit constexpr ZipRange(TRanges&&... ranges) : ranges_{std::forward<TRanges>(ranges)...} {}

  constexpr ConstIterator begin() const {
    return ConstIterator(ranges_ | star::transform([](const auto& r) { return std::begin(r); }) |
                         star::to_tuple);
  }
  constexpr ConstIterator end() const {
    return ConstIterator(ranges_ | star::transform([](const auto& r) { return std::end(r); }) |
                         star::to_tuple);
  }

  constexpr auto size() const {
    assert(ranges_ | star::transform([](const auto& r) { return std::size(r); }) |
           star::has_unique_value);
    return std::size(star::get_at<0>(ranges_));
  }

private:
  Tuple<TRanges...> ranges_;
};

template<typename... TRanges>
constexpr auto zip(TRanges&&... ranges) {
  return ZipRange<TRanges...>{std::forward<TRanges>(ranges)...};
}
} // namespace thes

#endif // INCLUDE_THESAUROS_RANGES_ZIP_HPP
