#include <iostream>
#include <numeric>

#include "thesauros/ranges.hpp"

int main() {
  constexpr auto r = thes::range(10);
  static_assert(r.contains(9));

  for (auto it = r.begin(); it != r.end(); ++it) {
    std::cout << *it << std::endl;
  }

  static_assert([r] {
    auto it = r.begin();
    return *(++it);
  }() == 1);
  static_assert([r] {
    auto it = r.begin();
    return *(it++);
  }() == 0);

  static_assert(r.begin()[4] == 4);
  static_assert(*(r.begin() + 4) == 4);
  static_assert(std::reduce(r.begin(), r.end()) == 45);
}
