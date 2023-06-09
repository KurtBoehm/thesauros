#include <iostream>

#include "thesauros/utility.hpp"

int main() {
  int a = 5;
  int b = 6;
  std::cout << a << "; " << b << '\n';

  thes::swap_or_equal<int>(a, b);
  std::cout << a << "; " << b << '\n';

  thes::swap_or_equal<int&>(a, a);
  std::cout << a << "; " << b << '\n';
}
