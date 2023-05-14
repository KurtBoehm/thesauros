#include <iomanip>
#include <iostream>
#include <numbers>
#include <sstream>

#include "thesauros/format.hpp"

#include "tools.hpp"

namespace test = thes::test;
namespace fmt = thes::fmt;

int main() {
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
}
