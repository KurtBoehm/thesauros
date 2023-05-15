#include <functional>
#include <iostream>
#include <ostream>
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

  thes::FixedThreadPool pool{2};
  thes::LinearExecutionPolicy expo{pool};

  thes::transform_inclusive_scan(
    expo, values.begin(), values.end(), scanned.begin(), std::plus<>{}, [](Type v) { return v; },
    Type{0});

  THES_ASSERT(scanned == scanned_ref);
}
