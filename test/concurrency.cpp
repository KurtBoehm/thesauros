#include <iostream>

#include "thesauros/execution.hpp"
#include "thesauros/test.hpp"

int main() {
  const auto phys = thes::physical_concurrency();
  THES_ASSERT(phys > 0);
  std::cout << phys << '\n';
}
