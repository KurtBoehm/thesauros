// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_RANGES_INDEX_TYPE_HPP
#define INCLUDE_THESAUROS_RANGES_INDEX_TYPE_HPP

#include <concepts>
#include <ranges>

namespace thes::ranges {
//--------------------------------------------------------------------------------------------------
// Deduction of the index type of a subscriptable range
//--------------------------------------------------------------------------------------------------

/**
 * A probe that converts to signed integers only, which makes it possible to detect whether
 * `operator[]` takes a signed index: Deduction for conversion function templates does not consider
 * integral conversions, so `T` is deduced to be the parameter type exactly.
 */
struct SignedIndexProbe {
  template<typename T>
  requires(std::signed_integral<T>)
  constexpr operator T() const; // NOLINT(*-explicit-conversions)
};
/** The unsigned counterpart of `SignedIndexProbe`. */
struct UnsignedIndexProbe {
  template<typename T>
  requires(std::unsigned_integral<T>)
  constexpr operator T() const; // NOLINT(*-explicit-conversions)
};

template<typename Range>
concept IsSignedIndexed = requires(Range& r) { r[SignedIndexProbe{}]; };
template<typename Range>
concept IsUnsignedIndexed = requires(Range& r) { r[UnsignedIndexProbe{}]; };

/**
 * The type to index `Range` with: The signed difference type if `operator[]` only accepts signed
 * indices, as is the case for C++20 views, and the unsigned size type otherwise, which covers
 * containers as well as ranges whose `operator[]` accepts either signedness.
 */
template<typename Range>
using RangeIndex = decltype([] {
  using Bare = std::remove_reference_t<Range>;
  using Size = std::decay_t<decltype(std::declval<Bare&>().size())>;

  if constexpr (IsSignedIndexed<Bare> && !IsUnsignedIndexed<Bare>) {
    if constexpr (std::ranges::range<Bare>) {
      return std::ranges::range_difference_t<Bare>{};
    } else {
      return std::make_signed_t<Size>{};
    }
  } else if constexpr (requires { typename Bare::size_type; }) {
    return typename Bare::size_type{};
  } else {
    return Size{};
  }
}());
} // namespace thes::ranges

#endif // INCLUDE_THESAUROS_RANGES_INDEX_TYPE_HPP
