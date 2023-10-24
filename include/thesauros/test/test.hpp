#ifndef INCLUDE_THESAUROS_TEST_TEST_HPP
#define INCLUDE_THESAUROS_TEST_TEST_HPP

#include <concepts>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <sstream>
#include <string_view>
#include <type_traits>

#include "thesauros/format.hpp"
#include "thesauros/io.hpp"
#include "thesauros/utility/empty.hpp"

namespace thes::test {
#ifdef NDEBUG
#define THES_ASSERT(expr)
#else
#define THES_ASSERT(expr) \
  ((expr) ? void(0) : ::thes::test::assert_fail(#expr, __FILE__, __LINE__, __func__, [] {}))
#endif

inline void assert_fail(const char* assertion, const char* file, unsigned int line,
                        const char* function, auto fun) {
  std::cerr << "Assertion “" << assertion << "” failed (" << function << " @ " << file << ":"
            << line << ")" << '\n';
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
         typename TStream = Empty>
inline constexpr bool range_eq(TRange1&& r1, TRange2&& r2, TEqual equal = {},
                               TStream&& stream = {}) {
  static_assert(detail::AreRanges<TRange1, TRange2>);

  if constexpr (detail::AreIterRanges<TRange1, TRange2>) {
    auto it1 = std::begin(r1);
    auto end1 = std::end(r1);
    auto it2 = std::begin(r2);
    auto end2 = std::end(r2);

    constexpr bool print = !std::same_as<TStream, Empty> && requires {
      stream << *it1;
      stream << *it2;
    };

    if constexpr (print) {
      stream << "range_eq: ";
    }
    std::size_t counter = 0;
    for (Delimiter delim{", "}; it1 != end1 && it2 != end2; ++it1, ++it2) {
      if constexpr (print) {
        stream << delim << thes::formatted(thes::fmt::rainbow_fg(counter++), *it1, "/", *it2);
      }
      if (!equal(*it1, *it2)) {
        if constexpr (print) {
          stream << '\n';
        }
        return false;
      }
    }
    if constexpr (print) {
      stream << '\n';
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
template<typename TStream = Empty>
inline bool string_eq(const std::string_view s1, const std::string_view s2, TStream&& stream = {}) {
  const bool eq = s1 == s2;

  if constexpr (!std::same_as<TStream, Empty>) {
    if (eq) {
      stream << formatted(fmt::fg_green, s1) << '\n';
    } else {
      for (Delimiter delim{", "}; char c : s1) {
        stream << delim << int(c);
      }
      stream << " vs. ";
      for (Delimiter delim{", "}; char c : s2) {
        stream << delim << int(c);
      }
      stream << '\n';

      stream << formatted(fmt::fg_red, s1) << (eq ? " == " : " != ") << formatted(fmt::fg_red, s2)
             << '\n';
    }
  }

  return eq;
}

template<bool tVerbose = true>
inline bool string_eq(const std::string_view s1, const auto&... v) {
  const auto s2 = (std::stringstream{} << ... << v).str();
  if constexpr (tVerbose) {
    return string_eq(s1, std::string_view{s2}, std::cout);
  } else {
    return string_eq(s1, std::string_view{s2});
  }
}
} // namespace thes::test

#endif // INCLUDE_THESAUROS_TEST_TEST_HPP
