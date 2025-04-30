// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <array>
#include <cstddef>

#include "thesauros/containers.hpp"
#include "thesauros/format.hpp"
#include "thesauros/test.hpp"

namespace test = thes::test;

int main() {
  thes::FixedBitset<4> bitset{
    false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  false,
    true,  false, true,  false, true,  false, true,  false, true,  false, true,  false, true,
    false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  false,
    true,  false, true,  false, true,  false, true,  false, true,  false, true,  false, true,
    false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  true,
  };
  std::array ref{
    false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  false,
    true,  false, true,  false, true,  false, true,  false, true,  false, true,  false, true,
    false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  false,
    true,  false, true,  false, true,  false, true,  false, true,  false, true,  false, true,
    false, true,  false, true,  false, true,  false, true,  false, true,  false, true,  true,
  };

  auto assert_eq = [&] {
    fmt::print("{}\n", bitset);
    THES_ASSERT(test::range_eq(bitset, ref));
  };

  auto set = [&](std::size_t idx) {
    bitset.set(idx);
    ref[idx] = true;
    assert_eq();
  };
  auto unset = [&](std::size_t idx) {
    bitset.unset(idx);
    ref[idx] = false;
    assert_eq();
  };
  auto get = [&](std::size_t idx) {
    const bool v1 = bitset.get(idx);
    const bool v2 = ref[idx];
    THES_ASSERT(v1 == v2);
    fmt::print("@{}: {}\n", idx, v1);
  };
  auto countr_one = [&] {
    const auto v1 = bitset.countr_one();
    const auto v2 = [&] {
      std::size_t count = 0;
      for (const auto v : ref) {
        if (!v) {
          break;
        }
        ++count;
      }
      return count;
    }();
    THES_ASSERT(v1 == v2);
    fmt::print("countr_one: {}\n", v1);
  };
  auto countr_zero = [&] {
    const auto v1 = bitset.countr_zero();
    const auto v2 = [&] {
      std::size_t count = 0;
      for (const auto v : ref) {
        if (v) {
          break;
        }
        ++count;
      }
      return count;
    }();
    THES_ASSERT(v1 == v2);
    fmt::print("countr_zero: {}\n", v1);
  };

  assert_eq();

  set(2);

  unset(bitset.size() - 1);
  get(1);
  countr_one();
  countr_zero();

  unset(1);
  countr_one();
  countr_zero();
}
