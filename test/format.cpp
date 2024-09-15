#include <filesystem>
#include <iostream>
#include <numbers>
#include <string>

#include "thesauros/format.hpp"
#include "thesauros/macropolis.hpp"

struct S {
  int i;

  friend std::ostream& operator<<(std::ostream& stream, const S& s) {
    return stream << "S(" << thes::formatted(thes::fmt::fg_cyan, s.i) << ")";
  }
};

template<>
struct fmt::formatter<S> : thes::SimpleFormatter<> {
  auto format(S s, format_context& ctx) const {
    // TODO Nested formatting does NOT work!
    return this->write_padded(ctx, [&](auto out) { return fmt::format_to(out, "S({})", s.i); });
  }
};

THES_CREATE_TYPE(SNAKE_CASE(Test), NORMAL_CONSTRUCTOR, (KEEP(a), std::filesystem::path),
                 (KEEP(c), std::string), (KEEP(d), double))

// TODO Nested formatting does NOT work!
#define USE_COUT true

int main() {
  fmt::print("{}\n", Test{std::filesystem::current_path(), "abc", 3.14});

  // Mix of manipulations and styles

  fmt::print(thes::fg_red, "test\n");
  fmt::print("{:.2}\n", std::numbers::pi);
  fmt::print(thes::fg_red, "{:.2}\n", std::numbers::pi);
  fmt::print(thes::fg_red | thes::bold, "{:.2}\n", std::numbers::pi);
  fmt::print(thes::fg_red | thes::bold, "{:08.6}\n", std::numbers::pi);
  fmt::print(thes::fg_red, "{:08.5}\n", -std::numbers::pi);
  fmt::print(thes::fg_red, "{:08.5f}\n", -2.5);
  fmt::print(thes::fg_black | thes::bg_bright_yellow, "{:08.4f}\n", -2.5);
  fmt::print(thes::fg_black | thes::bg_bright_yellow, "{:0>8}\n", S{3});

#if USE_COUT
  std::cout << thes::formatted(thes::fmt::zero_pad(8) | thes::fmt::precision(4) |
                                 thes::fmt::fg_black | thes::fmt::fixed |
                                 thes::fmt::bg_bright_yellow,
                               S{3})
            << '\n';
#endif

  fmt::print(thes::fg_yellow | thes::bg_blue, "That’s a yellow message on blue!\n");
  fmt::print("This should be normal again…\n");
  fmt::print(thes::bold | thes::italic | thes::fg_red, "That’s bold, italic and red!\n");
  fmt::print("This should be normal again…\n");
  fmt::print("{} {} a {} {}!\n", fmt::styled("This", thes::fg_blue),
             fmt::styled("is", thes::bold | thes::bg_yellow),
             fmt::styled("mixed", thes::fg_red | thes::underline),
             fmt::styled("message", thes::bold | thes::fg_bright_green));
  fmt::print("{} {} {} {} {} {}\n", fmt::styled("blue", thes::fg_blue | thes::italic),
             fmt::styled("red", thes::italic | thes::fg_red),
             fmt::styled("underline", thes::italic | thes::fg_red | thes::underline),
             fmt::styled("red", thes::italic | thes::fg_red),
             fmt::styled("green", thes::italic | thes::fg_green),
             fmt::styled("blue", thes::italic | thes::fg_blue | thes::bold));

#if USE_COUT
  std::cout << thes::formatted(thes::fmt::fg_blue | thes::fmt::italic, "blue ",
                               thes::formatted(thes::fmt::fg_red, "red ",
                                               thes::formatted(thes::fmt::underline, "underline"),
                                               " red",
                                               thes::formatted(thes::fmt::fg_green, " green")),
                               " ", thes::formatted(thes::fmt::bold, "blue"))
            << '\n';

  std::cout << thes::opt_formatted(thes::fmt::formatted_tag, thes::fmt::fg_red,
                                   "This should be formatted…")
            << '\n';
  std::cout << thes::opt_formatted(thes::fmt::unformatted_tag, thes::fmt::fg_red,
                                   "…and this should not!")
            << '\n';

  std::cout << thes::formatted(thes::fmt::fg_red, "Nested: ", S{3}, " after") << '\n';
  std::cout << "Normal?" << '\n';

  // TODO This is slightly buggy on Linux…
  {
    auto ctx = thes::fmt::make_context(std::cout);
    ctx.set(thes::fmt::fg_magenta | thes::fmt::bg_green | thes::fmt::italic | thes::fmt::width(5));
    std::cout << 123 << '\n';

    {
      auto ctx2 = thes::fmt::make_context(std::cout);
      ctx.set(thes::fmt::fg_yellow | thes::fmt::bg_none | thes::fmt::bold);
      std::cout << "234" << '\n';
    }

    std::cout << thes::formatted(thes::fmt::not_italic, "345") << '\n';
  }

  std::cout << "abc" << '\n';
#endif
}
