#include <iomanip>
#include <iostream>
#include <numbers>
#include <sstream>

#include "thesauros/format.hpp"
#include "thesauros/io.hpp"

#include "tools.hpp"

namespace test = thes::test;
namespace fmt = thes::fmt;

struct S {
  int i;

  friend std::ostream& operator<<(std::ostream& stream, const S& s) {
    return stream << "S(" << thes::formatted(fmt::fg_cyan, s.i) << ")";
  }
};

int main() {
  // Manipulations

  THES_ASSERT(test::stringeq("3.14", thes::formatted(fmt::precision(3), std::numbers::pi)));
  THES_ASSERT(test::stringeq("3.142", thes::formatted(fmt::precision(4), std::numbers::pi)));
  THES_ASSERT(test::stringeq("3.1416", thes::formatted(fmt::precision(5), std::numbers::pi)));
  THES_ASSERT(test::stringeq("3.14159265358979",
                             thes::formatted(fmt::full_precision<double>, std::numbers::pi)));
  THES_ASSERT(test::stringeq(
    "3.14159", thes::formatted(fmt::full_precision<float>, std::numbers::pi_v<float>)));
  THES_ASSERT(test::stringeq("2.000000", thes::formatted(fmt::fixed, 2.)));
  THES_ASSERT(test::stringeq("12.000000", thes::formatted(fmt::fixed, 12.)));

  THES_ASSERT(test::stringeq(
    "-02.5",
    thes::formatted(fmt::precision(5) | fmt::width(5) | fmt::fill('0') | fmt::internal, -2.5)));
  THES_ASSERT(test::stringeq(
    "-02.5",
    thes::formatted((fmt::precision(5) | fmt::width(5)) | (fmt::fill('0') | fmt::internal), -2.5)));
  const auto man1 = fmt::precision(5) | fmt::width(5);
  const auto man2 = fmt::fill('0') | fmt::internal;
  THES_ASSERT(test::stringeq("-02.5", thes::formatted(man1 | man2, -2.5)));
  THES_ASSERT(
    test::stringeq("-02.5", thes::formatted((fmt::precision(5) | fmt::width(5)) | man2, -2.5)));
  THES_ASSERT(
    test::stringeq("-02.5", thes::formatted(man1 | (fmt::fill('0') | fmt::internal), -2.5)));

  THES_ASSERT(
    test::stringeq("-012.5", thes::formatted(fmt::precision(5) | fmt::zero_pad(6), -12.5)));
  THES_ASSERT(
    test::stringeq("0012.5", thes::formatted(fmt::precision(5) | fmt::zero_pad(6), 12.5)));
  THES_ASSERT(test::stringeq("3.1416", thes::formatted(fmt::precision(5), std::numbers::pi)));
  THES_ASSERT(test::stringeq("3.141592654", thes::formatted(fmt::precision(10), std::numbers::pi)));

  // Mix of manipulations and styles

  std::cout << thes::formatted(fmt::fg_red, "test") << std::endl;
  std::cout << thes::formatted(fmt::precision(2), std::numbers::pi) << std::endl;
  std::cout << thes::formatted(fmt::fg_red | fmt::precision(2), std::numbers::pi) << std::endl;
  std::cout << thes::formatted(fmt::fg_red | fmt::bold | fmt::precision(2), std::numbers::pi)
            << std::endl;
  std::cout << thes::formatted(fmt::fg_red | fmt::bold | fmt::zero_pad(8), std::numbers::pi)
            << std::endl;
  std::cout << thes::formatted(fmt::zero_pad(8) | fmt::fg_red | fmt::bold, std::numbers::pi)
            << std::endl;
  std::cout << thes::formatted(fmt::zero_pad(8) | fmt::fg_red | fmt::precision(5),
                               -std::numbers::pi)
            << std::endl;
  std::cout << thes::formatted(fmt::zero_pad(8) | fmt::precision(4) | fmt::fg_red | fmt::fixed,
                               -2.5)
            << std::endl;
  std::cout << thes::formatted(fmt::zero_pad(8) | fmt::precision(4) | fmt::fg_black | fmt::fixed |
                                 fmt::bg_bright_yellow,
                               -2.5)
            << std::endl;
  std::cout << thes::formatted(fmt::zero_pad(8) | fmt::precision(4) | fmt::fg_black | fmt::fixed |
                                 fmt::bg_bright_yellow,
                               S{3})
            << std::endl;

  {
    auto ctx = fmt::make_context(std::cout);
    ctx.set(fmt::fg_red | fmt::bg_blue | fmt::italic | fmt::width(5));
    std::cout << 123 << std::endl;

    {
      auto ctx2 = fmt::make_context(std::cout);
      ctx.set(fmt::fg_yellow | fmt::bg_none | fmt::bold);
      std::cout << "234" << std::endl;
    }

    std::cout << thes::formatted(fmt::not_italic, "345") << std::endl;
  }
  std::cout << "abc" << std::endl;
}
