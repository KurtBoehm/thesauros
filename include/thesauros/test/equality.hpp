// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_TEST_EQUALITY_HPP
#define INCLUDE_THESAUROS_TEST_EQUALITY_HPP

#include <cstdlib>
#include <functional>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <utility>

#include "thesauros/format.hpp"
#include "thesauros/functional/no-op.hpp"
#include "thesauros/io.hpp"
#include "thesauros/math/integer-cast.hpp"
#include "thesauros/ranges/index-type.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes::test {
namespace detail {
template<typename Range>
concept IsIterRange = requires(Range&& r) {
  std::ranges::begin(r);
  std::ranges::end(r);
};
template<typename Range>
concept IsAccessRange = requires(Range&& r) { r[r.size()]; };

template<typename Range1, typename Range2>
concept AreIterRanges = IsIterRange<Range1> && IsIterRange<Range2>;
template<typename Range1, typename Range2>
concept AreAccessRanges = IsAccessRange<Range1> && IsAccessRange<Range2>;

template<typename Range1, typename Range2>
concept AreRanges = AreIterRanges<Range1, Range2> || AreAccessRanges<Range1, Range2>;
} // namespace detail

template<typename Range1, typename Range2, typename Equal = std::equal_to<>,
         typename Print = NoOp<>>
constexpr bool range_eq(Range1&& r1, Range2&& r2, // NOLINT(*-missing-std-forward)
                        Equal equal = {}, Print printer = {}) {
  static_assert(detail::AreRanges<Range1, Range2>);

  if constexpr (detail::AreIterRanges<Range1, Range2>) {
    auto it1 = std::ranges::begin(r1);
    auto end1 = std::ranges::end(r1);
    auto it2 = std::ranges::begin(r2);
    auto end2 = std::ranges::end(r2);

    constexpr bool print = !AnyNoOp<Print>;
    if constexpr (print) {
      printer("range_eq: ");
    }
    std::size_t counter = 0;
    for (Delimiter delim{", "}; it1 != end1 && it2 != end2; ++it1, ++it2) {
      if constexpr (print) {
        printer("{}", delim);
        printer(rainbow_fg(counter++), "{}/{}", *it1, *it2);
      }
      if (!equal(*it1, *it2)) {
        if constexpr (print) {
          printer("\n");
        }
        return false;
      }
    }
    if constexpr (print) {
      printer("\n");
    }
    return (it1 == end1) == (it2 == end2);
  }
  if constexpr (detail::AreAccessRanges<Range1, Range2>) {
    const auto size1 = r1.size();
    const auto size2 = r2.size();
    if (std::cmp_not_equal(size1, size2)) {
      return false;
    }

    using Index1 = ranges::RangeIndex<Range1>;
    using Index2 = ranges::RangeIndex<Range2>;

    for (std::decay_t<decltype(size1)> i = 0; i < size1; ++i) {
      if (!equal(r1[*safe_cast<Index1>(i)], r2[*safe_cast<Index2>(i)])) {
        return false;
      }
    }
    return true;
  }
}
template<typename Printer>
inline bool string_eq(const std::string_view s1, const std::string_view s2, Printer printer) {
  const bool eq = s1 == s2;

  if constexpr (!AnyNoOp<Printer>) {
    if (eq) {
      printer(fg_green, "{}\n", s1);
    } else {
      for (Delimiter delim{", "}; char c : s1) {
        printer("{}{}", delim, int(c));
      }
      printer(" vs. ");
      for (Delimiter delim{", "}; char c : s2) {
        printer("{}{}", delim, int(c));
      }
      printer("\n");

      printer("{} {} {}\n", fmt::styled(s1, fg_red), eq ? "==" : "!=", fmt::styled(s2, fg_red));
    }
  }

  return eq;
}

struct StringEqPrinter {
  template<typename... Args>
  void operator()(fmt::format_string<Args...> fmt, Args&&... args) {
    fmt::print(fmt, std::forward<Args>(args)...);
  }
  template<typename... Args>
  void operator()(const fmt::text_style& ts, fmt::format_string<Args...> fmt, Args&&... args) {
    fmt::print(ts, fmt, std::forward<Args>(args)...);
  }
};

template<bool Verbose = true>
inline bool string_eq(const std::string_view s1, const auto& v, BoolTag<Verbose> verbose = {}) {
  const auto s2 = fmt::format("{}", v);
  if constexpr (verbose) {
    return string_eq(s1, std::string_view{s2}, StringEqPrinter{});
  }
  return string_eq(s1, std::string_view{s2}, NoOp{});
}
} // namespace thes::test

#endif // INCLUDE_THESAUROS_TEST_EQUALITY_HPP
