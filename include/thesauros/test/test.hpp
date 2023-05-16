#ifndef INCLUDE_THESAUROS_TEST_TEST_HPP
#define INCLUDE_THESAUROS_TEST_TEST_HPP

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string_view>

#include "thesauros/format.hpp"

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
            << line << ")" << std::endl;
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
concept IsAccessRange = requires(TRange&& r, std::size_t i) {
  r.size();
  r[i];
};

template<typename TRange1, typename TRange2>
concept AreIterRanges = IsIterRange<TRange1> && IsIterRange<TRange2>;
template<typename TRange1, typename TRange2>
concept AreAccessRanges = IsAccessRange<TRange1> && IsAccessRange<TRange2>;

template<typename TRange1, typename TRange2>
concept AreRanges = AreIterRanges<TRange1, TRange2> || AreAccessRanges<TRange1, TRange2>;
} // namespace detail

template<typename TRange1, typename TRange2>
inline constexpr bool range_eq(TRange1&& r1, TRange2&& r2) {
  static_assert(detail::AreRanges<TRange1, TRange2>);

  if constexpr (detail::AreIterRanges<TRange1, TRange2>) {
    auto it1 = std::begin(r1);
    auto end1 = std::end(r1);
    auto it2 = std::begin(r2);
    auto end2 = std::end(r2);

    constexpr bool can_print = requires {
      std::cout << *it1;
      std::cout << *it2;
    };

    if constexpr (can_print) {
      std::cout << "range_eq:";
    }
    for (; it1 != end1 && it2 != end2; ++it1, ++it2) {
      if constexpr (can_print) {
        std::cout << ' ' << *it1 << "/" << *it2;
      }
      if (*it1 != *it2) {
        if constexpr (can_print) {
          std::cout << std::endl;
        }
        return false;
      }
    }
    if constexpr (can_print) {
      std::cout << std::endl;
    }
    return (it1 == end1) == (it2 == end2);
  }
  if constexpr (detail::AreAccessRanges<TRange1, TRange2>) {
    const std::size_t size1{r1.size()};
    const std::size_t size2{r2.size()};
    if (size1 != size2) {
      return false;
    }
    for (std::size_t i = 0; i < size1; ++i) {
      if (r1[i] != r2[i]) {
        return false;
      }
    }
    return true;
  }
}

inline bool string_eq(const std::string_view s1, const std::string_view s2) {
  const bool eq = s1 == s2;
  if (eq) {
    std::cout << formatted(fmt::fg_green, s1) << std::endl;
  } else {
    for (char c : s1) {
      std::cout << int(c) << ' ';
    }
    std::cout << "vs.";
    for (char c : s2) {
      std::cout << ' ' << int(c);
    }
    std::cout << std::endl;

    std::cout << formatted(fmt::fg_red, s1) << (eq ? " == " : " != ") << formatted(fmt::fg_red, s2)
              << std::endl;
  }
  return eq;
}
inline bool string_eq(const std::string_view s1, const auto&... v) {
  const auto s2 = (std::stringstream{} << ... << v).str();
  return string_eq(s1, std::string_view{s2});
}
} // namespace thes::test

#endif // INCLUDE_THESAUROS_TEST_TEST_HPP
