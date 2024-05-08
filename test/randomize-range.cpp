#include <algorithm>
#include <numeric>
#include <random>

#include "thesauros/thesauros.hpp"

using Size = unsigned int;

void run(const Size size) {
  thes::DynamicArrayDefault<Size> arr(size);
  std::iota(arr.begin(), arr.end(), Size{0});
  thes::DynamicArrayDefault<Size> perm(size);
  thes::RangeRandomizer rand{size, std::mt19937_64{size}};
  std::transform(arr.begin(), arr.end(), perm.begin(), [&](Size i) { return rand.transform(i); });
  if (size <= 48) {
    fmt::print("{}\n", perm);
  }
  std::sort(perm.begin(), perm.end());
  THES_ASSERT(std::equal(arr.begin(), arr.end(), perm.begin(), perm.end()));
}

int main() {
  for (Size size : thes::range<Size>(100)) {
    run(size);
  }
  run(1U << 10U);
  run((1U << 13U) + (1U << 3U));
  run(0x3a745cf);
}
