#ifndef INCLUDE_THESAUROS_TEST_TEST_HPP
#define INCLUDE_THESAUROS_TEST_TEST_HPP

#include <cstdlib>
#include <functional>
#include <source_location>
#include <string_view>
#include <type_traits>

#include "thesauros/format.hpp"
#include "thesauros/functional/no-op.hpp"
#include "thesauros/io.hpp"
#include "thesauros/utility/value-tag.hpp"

namespace thes::test {
#define THES_ALWAYS_ASSERT(expr) ((expr) ? void(0) : ::thes::test::assert_fail(#expr, [] {}))

#ifdef NDEBUG
#define THES_ASSERT(expr)
#else
#define THES_ASSERT(expr) THES_ALWAYS_ASSERT(expr)
#endif

inline void assert_fail(const char* assertion, auto fun,
                        const std::source_location location = std::source_location::current()) {
  fmt::print(stderr, "Assertion “{}” failed in {} @ {}:{}:{}\n", assertion,
             location.function_name(), location.file_name(), location.line(), location.column());
  fun();
  std::abort();
}

namespace detail {
template<typename TRange>
concept IsIterRange = requires(TRange&& r) {
  std::begin(r);
  std::end(r);
};
template<typename TRange>
concept IsAccessRange = requires(TRange&& r) { r[r.size()]; };

template<typename TRange1, typename TRange2>
concept AreIterRanges = IsIterRange<TRange1> && IsIterRange<TRange2>;
template<typename TRange1, typename TRange2>
concept AreAccessRanges = IsAccessRange<TRange1> && IsAccessRange<TRange2>;

template<typename TRange1, typename TRange2>
concept AreRanges = AreIterRanges<TRange1, TRange2> || AreAccessRanges<TRange1, TRange2>;
} // namespace detail

template<typename TRange1, typename TRange2, typename TEqual = std::equal_to<>,
         typename TPrint = NoOp<>>
constexpr bool range_eq(TRange1&& r1, TRange2&& r2, TEqual equal = {}, TPrint printer = {}) {
  static_assert(detail::AreRanges<TRange1, TRange2>);

  if constexpr (detail::AreIterRanges<TRange1, TRange2>) {
    auto it1 = std::begin(r1);
    auto end1 = std::end(r1);
    auto it2 = std::begin(r2);
    auto end2 = std::end(r2);

    constexpr bool print = !AnyNoOp<TPrint>;
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
  if constexpr (detail::AreAccessRanges<TRange1, TRange2>) {
    const auto size1{r1.size()};
    const auto size2{r2.size()};
    if (size1 != size2) {
      return false;
    }
    for (std::decay_t<decltype(size1)> i = 0; i < size1; ++i) {
      if (r1[i] != r2[i]) {
        return false;
      }
    }
    return true;
  }
}
template<typename TPrint>
inline bool string_eq(const std::string_view s1, const std::string_view s2, TPrint printer) {
  const bool eq = s1 == s2;

  if constexpr (!AnyNoOp<TPrint>) {
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
  template<typename... TArgs>
  void operator()(fmt::format_string<TArgs...> fmt, TArgs&&... args) {
    fmt::print(fmt, std::forward<TArgs>(args)...);
  }
  template<typename... TArgs>
  void operator()(const fmt::text_style& ts, fmt::format_string<TArgs...> fmt, TArgs&&... args) {
    fmt::print(ts, fmt, std::forward<TArgs>(args)...);
  }
};

template<bool tVerbose = true>
inline bool string_eq(const std::string_view s1, const auto& v, BoolTag<tVerbose> verbose = {}) {
  const auto s2 = fmt::format("{}", v);
  if constexpr (verbose) {
    return string_eq(s1, std::string_view{s2}, StringEqPrinter{});
  }
  return string_eq(s1, std::string_view{s2}, NoOp{});
}
} // namespace thes::test

#endif // INCLUDE_THESAUROS_TEST_TEST_HPP
