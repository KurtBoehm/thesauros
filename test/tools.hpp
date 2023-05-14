#ifndef TEST_TOOLS_HPP
#define TEST_TOOLS_HPP

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string_view>

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

inline constexpr bool stringeq(const std::string_view s1, const std::string_view s2) {
  return s1 == s2;
}
inline constexpr bool stringeq(const auto& v, const std::string_view s2) {
  return (std::stringstream{} << v).str() == s2;
}
} // namespace thes::test

#endif // TEST_TOOLS_HPP
