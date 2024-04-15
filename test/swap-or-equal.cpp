#include "thesauros/format.hpp"
#include "thesauros/utility.hpp"

int main() {
  int a = 5;
  int b = 6;
  fmt::print("{}; {}\n", a, b);

  thes::swap_or_equal<int>(a, b);
  fmt::print("{}; {}\n", a, b);

  thes::swap_or_equal<int&>(a, a);
  fmt::print("{}; {}\n", a, b);
}
