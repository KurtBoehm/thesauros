#include <cstdint>
#include <iostream>
#include <sstream>

#include "thesauros/containers.hpp"
#include "thesauros/ranges.hpp"
#include "thesauros/test.hpp"

namespace test = thes::test;

int main() {
  thes::DynamicBitset<std::uint32_t> bitset{};
  THES_ASSERT(bitset.chunk_num() == 0 && bitset.size() == 0);
  for (const auto i : thes::range<std::size_t>(32)) {
    bitset.push_back((i % 2) != 0);
  }
  THES_ASSERT(bitset.chunk_num() == 1 && bitset.countr_one() == 0 && bitset.countr_zero() == 1);
  THES_ASSERT(test::string_eq("10101010101010101010101010101010", bitset));

  bitset.push_back(true);
  bitset.set(0);
  THES_ASSERT(bitset.chunk_num() == 2 && bitset.countr_one() == 2 && bitset.countr_zero() == 0);
  THES_ASSERT(test::string_eq("110101010101010101010101010101011", bitset));

  for (const auto i : thes::range<std::size_t>(31)) {
    bitset.push_back((i % 2) != 0);
  }
  THES_ASSERT(bitset.chunk_num() == 2 && bitset.countr_one() == 2 && bitset.countr_zero() == 0);
  THES_ASSERT(
    test::string_eq("0101010101010101010101010101010110101010101010101010101010101011", bitset));

  bitset.push_back(true);
  THES_ASSERT(bitset.chunk_num() == 3 && bitset.countr_one() == 2 && bitset.countr_zero() == 0);
  THES_ASSERT(
    test::string_eq("10101010101010101010101010101010110101010101010101010101010101011", bitset));
}
