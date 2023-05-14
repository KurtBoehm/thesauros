#ifndef TEST_TOOLS_HPP
#define TEST_TOOLS_HPP

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

inline constexpr bool rangeq(const auto& r1, const auto& r2) {
  {
    auto it1 = std::begin(r1);
    auto end1 = std::end(r1);
    auto it2 = std::begin(r2);
    auto end2 = std::end(r2);

    for (; it1 != end1 && it2 != end2; ++it1, ++it2) {
      if (*it1 != *it2) {
        return false;
      }
    }
    return (it1 == end1) == (it2 == end2);
  }
  if constexpr (requires(std::size_t i) {
                  r1.size();
                  r2.size();
                  r1[i];
                  r2[i];
                }) {
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

inline bool stringeq(const std::string_view s1, const std::string_view s2) {
  namespace fmt = thes::fmt;

  const bool eq = s1 == s2;
  if (eq) {
    std::cout << fmt::set(fmt::fg_green, s1) << std::endl;
  } else {
    std::cout << fmt::set(fmt::fg_red, s1) << (eq ? " == " : " != ") << fmt::set(fmt::fg_red, s2)
              << std::endl;
  }
  return eq;
}
inline bool stringeq(const std::string_view s1, const auto&... v) {
  const auto s2 = (std::stringstream{} << ... << v).str();
  return stringeq(s1, std::string_view{s2});
}
} // namespace thes::test

#endif // TEST_TOOLS_HPP
