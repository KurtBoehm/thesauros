#include "thesauros/execution.hpp"
#include "thesauros/format.hpp"
#include "thesauros/test.hpp"

int main() {
  const auto phys = thes::physical_concurrency();
  THES_ASSERT(phys > 0);
  fmt::println("{}", phys);
}
