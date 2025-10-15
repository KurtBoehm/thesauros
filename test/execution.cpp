// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <functional>
#include <vector>

#include "thesauros/algorithms.hpp"
#include "thesauros/execution.hpp"
#include "thesauros/resources.hpp"
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

  fmt::print("logical: {}\n", thes::CpuInfo::logical());
  fmt::print("physical: {}\n", thes::CpuInfo::physical());
  fmt::print("physical 0/2: {}\n", thes::CpuInfo::physical_part(0, 2));
}
