#include <cstddef>
#include <iostream>
#include <type_traits>
#include <vector>

#include "thesauros/containers.hpp"

#include "tools.hpp"

namespace test = thes::test;

constexpr int run() {
  thes::StaticBitset bitset{
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
    if (!std::is_constant_evaluated()) {
      std::cout << bitset << std::endl;
    }
    THES_ASSERT(test::rangeq(bitset, ref));
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
    if (!std::is_constant_evaluated()) {
      std::cout << "@" << idx << ": " << v1 << std::endl;
    }
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
    if (!std::is_constant_evaluated()) {
      std::cout << "countr_one: " << v1 << std::endl;
    }
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
    if (!std::is_constant_evaluated()) {
      std::cout << "countr_zero: " << v1 << std::endl;
    }
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

  return 0;
}

int main() {
  static_assert(run() == 0);
  return run();
}
