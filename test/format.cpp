#include <iostream>
#include <numbers>

#include "thesauros/format.hpp"
#include "thesauros/test.hpp"

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

  THES_ASSERT(test::string_eq("3.14", thes::formatted(fmt::precision(3), std::numbers::pi)));
  THES_ASSERT(test::string_eq("3.142", thes::formatted(fmt::precision(4), std::numbers::pi)));
  THES_ASSERT(test::string_eq("3.1416", thes::formatted(fmt::precision(5), std::numbers::pi)));
  THES_ASSERT(test::string_eq("3.14159265358979",
                              thes::formatted(fmt::full_precision<double>, std::numbers::pi)));
  THES_ASSERT(test::string_eq(
    "3.14159", thes::formatted(fmt::full_precision<float>, std::numbers::pi_v<float>)));
  THES_ASSERT(test::string_eq("2.000000", thes::formatted(fmt::fixed, 2.)));
  THES_ASSERT(test::string_eq("12.000000", thes::formatted(fmt::fixed, 12.)));

  THES_ASSERT(test::string_eq(
    "-02.5",
    thes::formatted(fmt::precision(5) | fmt::width(5) | fmt::fill('0') | fmt::internal, -2.5)));
  THES_ASSERT(test::string_eq(
    "-02.5",
    thes::formatted((fmt::precision(5) | fmt::width(5)) | (fmt::fill('0') | fmt::internal), -2.5)));
  const auto man1 = fmt::precision(5) | fmt::width(5);
  const auto man2 = fmt::fill('0') | fmt::internal;
  THES_ASSERT(test::string_eq("-02.5", thes::formatted(man1 | man2, -2.5)));
  THES_ASSERT(
    test::string_eq("-02.5", thes::formatted((fmt::precision(5) | fmt::width(5)) | man2, -2.5)));
  THES_ASSERT(
    test::string_eq("-02.5", thes::formatted(man1 | (fmt::fill('0') | fmt::internal), -2.5)));

  THES_ASSERT(
    test::string_eq("-012.5", thes::formatted(fmt::precision(5) | fmt::zero_pad(6), -12.5)));
  THES_ASSERT(
    test::string_eq("0012.5", thes::formatted(fmt::precision(5) | fmt::zero_pad(6), 12.5)));
  THES_ASSERT(test::string_eq("3.1416", thes::formatted(fmt::precision(5), std::numbers::pi)));
  THES_ASSERT(
    test::string_eq("3.141592654", thes::formatted(fmt::precision(10), std::numbers::pi)));

  // Mix of manipulations and styles

  std::cout << thes::formatted(fmt::fg_red, "test") << '\n';
  std::cout << thes::formatted(fmt::precision(2), std::numbers::pi) << '\n';
  std::cout << thes::formatted(fmt::fg_red | fmt::precision(2), std::numbers::pi) << '\n';
  std::cout << thes::formatted(fmt::fg_red | fmt::bold | fmt::precision(2), std::numbers::pi)
            << '\n';
  std::cout << thes::formatted(fmt::fg_red | fmt::bold | fmt::zero_pad(8), std::numbers::pi)
            << '\n';
  std::cout << thes::formatted(fmt::zero_pad(8) | fmt::fg_red | fmt::bold, std::numbers::pi)
            << '\n';
  std::cout << thes::formatted(fmt::zero_pad(8) | fmt::fg_red | fmt::precision(5),
                               -std::numbers::pi)
            << '\n';
  std::cout << thes::formatted(fmt::zero_pad(8) | fmt::precision(4) | fmt::fg_red | fmt::fixed,
                               -2.5)
            << '\n';
  std::cout << thes::formatted(fmt::zero_pad(8) | fmt::precision(4) | fmt::fg_black | fmt::fixed |
                                 fmt::bg_bright_yellow,
                               -2.5)
            << '\n';
  std::cout << thes::formatted(fmt::zero_pad(8) | fmt::precision(4) | fmt::fg_black | fmt::fixed |
                                 fmt::bg_bright_yellow,
                               S{3})
            << '\n';

  std::cout << thes::formatted(fmt::fg_yellow | fmt::bg_blue, "That’s a yellow message on blue!")
            << '\n';
  std::cout << "This should be normal again…" << '\n';
  std::cout << thes::formatted(fmt::bold | fmt::italic | fmt::fg_red,
                               "That’s bold, italic and red!")
            << '\n';
  std::cout << "This should be normal again…" << '\n';
  std::cout << thes::formatted(fmt::fg_blue, "This") << " "
            << thes::formatted(fmt::bold | fmt::bg_yellow, "is") << " a "
            << thes::formatted(fmt::fg_red | fmt::underline, "mixed") << " "
            << thes::formatted(fmt::bold | fmt::fg_bright_green, "message") << "!" << '\n';

  std::cout << thes::formatted(fmt::fg_blue | fmt::italic, "blue ",
                               thes::formatted(fmt::fg_red, "red ",
                                               thes::formatted(fmt::underline, "underline"), " red",
                                               thes::formatted(fmt::fg_green, " green")),
                               " ", thes::formatted(fmt::bold, "blue"))
            << '\n';

  std::cout << thes::opt_formatted(fmt::formatted_tag, fmt::fg_red, "This should be formatted…")
            << '\n';
  std::cout << thes::opt_formatted(fmt::unformatted_tag, fmt::fg_red, "…and this should not!")
            << '\n';

  std::cout << thes::formatted(fmt::fg_red, "Nested: ", S{3}, " after") << '\n';
  std::cout << "Normal?" << '\n';

  // TODO This is slightly buggy on Linux…
  {
    auto ctx = fmt::make_context(std::cout);
    ctx.set(fmt::fg_magenta | fmt::bg_green | fmt::italic | fmt::width(5));
    std::cout << 123 << '\n';

    {
      auto ctx2 = fmt::make_context(std::cout);
      ctx.set(fmt::fg_yellow | fmt::bg_none | fmt::bold);
      std::cout << "234" << '\n';
    }

    std::cout << thes::formatted(fmt::not_italic, "345") << '\n';
  }

  std::cout << "abc" << '\n';
}
