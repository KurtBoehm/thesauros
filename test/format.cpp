#include <iomanip>
#include <iostream>
#include <numbers>
#include <sstream>

#include "thesauros/format.hpp"
#include "thesauros/io.hpp"

#include "tools.hpp"

namespace test = thes::test;
namespace fmt = thes::fmt;
namespace fmt2 = thes::fmt2;

struct S {
  int i;

  template<thes::OutStream TStream>
  friend TStream& operator<<(TStream& stream, const S& s) {
    return stream << "S(" << fmt::styled_args(fmt::fg_cyan, s.i) << ")";
  }
};

int main() {
  // Manipulations

  THES_ASSERT(test::stringeq("3.14", fmt::manip_args(fmt::precision(3), std::numbers::pi)));
  THES_ASSERT(test::stringeq("3.142", fmt::manip_args(fmt::precision(4), std::numbers::pi)));
  THES_ASSERT(test::stringeq("3.1416", fmt::manip_args(fmt::precision(5), std::numbers::pi)));
  THES_ASSERT(test::stringeq("3.14159265358979",
                             fmt::manip_args(fmt::digit_precision<double>(), std::numbers::pi)));
  THES_ASSERT(test::stringeq(
    "3.14159", fmt::manip_args(fmt::digit_precision<float>(), std::numbers::pi_v<float>)));
  THES_ASSERT(test::stringeq("2.000000", fmt::manip_args(fmt::fixed(), 2.)));
  THES_ASSERT(test::stringeq("12.000000", fmt::manip_args(fmt::fixed(), 12.)));

  THES_ASSERT(test::stringeq(
    "-02.5",
    fmt::manip_args(fmt::precision(5) | fmt::width(5) | fmt::fill('0') | fmt::internal(), -2.5)));
  THES_ASSERT(test::stringeq("-02.5", fmt::manip_args((fmt::precision(5) | fmt::width(5)) |
                                                        (fmt::fill('0') | fmt::internal()),
                                                      -2.5)));
  const auto man1 = fmt::precision(5) | fmt::width(5);
  const auto man2 = fmt::fill('0') | fmt::internal();
  THES_ASSERT(test::stringeq("-02.5", fmt::manip_args(man1 | man2, -2.5)));
  THES_ASSERT(
    test::stringeq("-02.5", fmt::manip_args((fmt::precision(5) | fmt::width(5)) | man2, -2.5)));
  THES_ASSERT(
    test::stringeq("-02.5", fmt::manip_args(man1 | (fmt::fill('0') | fmt::internal()), -2.5)));

  THES_ASSERT(
    test::stringeq("-012.5", fmt::manip_args(fmt::precision(5) | fmt::zero_pad(6), -12.5)));
  THES_ASSERT(
    test::stringeq("0012.5", fmt::manip_args(fmt::precision(5) | fmt::zero_pad(6), 12.5)));
  THES_ASSERT(test::stringeq("3.1416", fmt::manip_args(fmt::precision(5), std::numbers::pi)));
  THES_ASSERT(test::stringeq("3.141592654", fmt::manip_args(fmt::precision(10), std::numbers::pi)));

  // Mix of manipulations and styles

  std::cout << fmt::set(fmt::fg_red, "test") << std::endl;
  std::cout << fmt::set(fmt::precision(2), std::numbers::pi) << std::endl;
  std::cout << fmt::set(fmt::fg_red | fmt::precision(2), std::numbers::pi) << std::endl;
  std::cout << fmt::set(fmt::fg_red | fmt::bold | fmt::precision(2), std::numbers::pi) << std::endl;
  std::cout << fmt::set(fmt::fg_red | fmt::bold | fmt::zero_pad(8), std::numbers::pi) << std::endl;
  std::cout << fmt::set(fmt::zero_pad(8) | fmt::fg_red | fmt::bold, std::numbers::pi) << std::endl;
  std::cout << fmt::set(fmt::zero_pad(8) | fmt::fg_red | fmt::precision(5), -std::numbers::pi)
            << std::endl;
  std::cout << fmt::set(fmt::zero_pad(8) | fmt::precision(4) | fmt::fg_red | fmt::fixed(), -2.5)
            << std::endl;
  std::cout << fmt::set(fmt::zero_pad(8) | fmt::precision(4) | fmt::fg_black | fmt::fixed() |
                          fmt::bg_bright_yellow,
                        -2.5)
            << std::endl;
  std::cout << fmt::set(fmt::zero_pad(8) | fmt::precision(4) | fmt::fg_black | fmt::fixed() |
                          fmt::bg_bright_yellow,
                        S{3})
            << std::endl;

  {
    auto ctx = fmt2::StyleContext{std::cout};
    ctx.set(fmt2::fg_red, fmt2::bg_blue, fmt2::italic);
    std::cout << "123" << std::endl;

    {
      auto ctx2 = fmt2::make_context(std::cout);
      ctx.set(fmt2::fg_yellow | fmt2::bg_none | fmt2::bold);
      std::cout << "234" << std::endl;
    }

    {
      auto ctx2 = fmt2::StyleContext{std::cout};
      std::cout << fmt2::set(fmt2::not_italic, "345") << std::endl;
    }
  }
  std::cout << "abc" << std::endl;
}
