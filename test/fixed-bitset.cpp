#include <array>
#include <cstddef>
#include <iostream>
#include <type_traits>
#include <vector>

#include "thesauros/containers.hpp"
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
    std::cout << bitset << '\n';
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
    std::cout << "@" << idx << ": " << v1 << '\n';
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
    std::cout << "countr_one: " << v1 << '\n';
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
    std::cout << "countr_zero: " << v1 << '\n';
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
