// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <filesystem>
#include <numbers>
#include <string>

#include "thesauros/format.hpp"
#include "thesauros/macropolis.hpp"

struct S {
  int i;
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
}
