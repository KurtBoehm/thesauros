#include <functional>
#include <vector>

#include "thesauros/algorithms.hpp"
#include "thesauros/execution.hpp"
#include "thesauros/test.hpp"

int main() {
  using Type = int;

  std::vector<Type> values{5, 8, 9, 4, 2, -10, 22};
  std::vector<Type> scanned(values.size());
  std::vector<Type> scanned_ref{};
  for (Type accum = 0; const Type v : values) {
    accum += v;
    scanned_ref.push_back(accum);
  }

  auto op = [&](auto pool) {
    thes::LinearExecutionPolicy expo{pool};

    thes::transform_inclusive_scan(
      expo, values.begin(), values.end(), scanned.begin(), std::plus<>{}, [](Type v) { return v; },
      Type{0});

    THES_ASSERT(scanned == scanned_ref);
  };
  op(thes::FixedStdThreadPool{2});
  op(thes::FixedOpenMpThreadPool{2});
}
