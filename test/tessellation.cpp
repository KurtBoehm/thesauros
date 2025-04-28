#include <array>
#include <functional>

#include "thesauros/format.hpp"
#include "thesauros/math.hpp"
#include "thesauros/static-ranges.hpp"
#include "thesauros/test.hpp"

int main() {
  using Int = unsigned;
  for (Int i = 1; i < 256; ++i) {
    const auto tess = thes::box_tesselate(i, std::array<Int, 3>{1451, 1530, 1530});
    THES_ALWAYS_ASSERT((tess | thes::star::left_reduce(std::multiplies{})) == i);
    fmt::print("{}: {}\n", i, tess);
  }
}
