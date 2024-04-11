#include <fmt/core.h>

#include "thesauros/utility.hpp"

int main() {
  int a = 5;
  int b = 6;
  fmt::println("{}; {}", a, b);

  thes::swap_or_equal<int>(a, b);
  fmt::println("{}; {}", a, b);

  thes::swap_or_equal<int&>(a, a);
  fmt::println("{}; {}", a, b);
}
