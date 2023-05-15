#include <algorithm>
#include <climits>
#include <cstdint>
#include <iostream>
#include <vector>

#include "thesauros/containers.hpp"
#include "thesauros/utility.hpp"

#include "thesauros/test.hpp"

namespace test = thes::test;

using ByteInt = thes::ByteInteger<3>;
using UInt = ByteInt::Unsigned;
using Mbi = thes::MultiByteIntegers<ByteInt, 4>;

inline constexpr UInt modulus = UInt{1} << (CHAR_BIT * ByteInt::byte_num);

int main() {
  Mbi integers{13};
  std::vector<UInt> vec{13};

  auto elem_assert = [&integers, &vec] {
    std::cout << '{' << thes::range_print(integers) << "}, [" << thes::range_print(vec) << ']'
              << std::endl;
    THES_ASSERT(test::range_eq(integers, vec));
  };
  auto push_back = [&integers, &vec](UInt v) {
    integers.push_back(v);
    vec.push_back(v % modulus);
  };
  auto assign = [&integers, &vec](std::size_t i, UInt v) {
    integers[i] = v;
    vec[i] = v % modulus;
  };
  auto pop_back = [&integers, &vec]() {
    integers.pop_back();
    vec.pop_back();
  };

  elem_assert();

  for (UInt i :
       {UInt{1}, UInt{2}, UInt{3}, UInt{1ULL << 16ULL}, UInt{1ULL << 20ULL}, UInt{1ULL << 24ULL}}) {
    push_back(i);
    elem_assert();
  }

  assign(1, 4);
  assign(4, (1ULL << 24ULL) + (1ULL << 15ULL));
  elem_assert();

  pop_back();
  pop_back();
  elem_assert();

  for (auto&& it : integers) {
    it = it + 1;
  }
  for (auto& v : vec) {
    ++v;
  }
  elem_assert();

  {
    using std::swap;

    swap(integers[1], integers[3]);
    THES_ASSERT(integers[1] == vec[3]);
    THES_ASSERT(integers[3] == vec[1]);
    swap(vec[1], vec[3]);
    elem_assert();

    const UInt v0 = 12;
    UInt v = v0;
    swap(integers[0], v);
    THES_ASSERT(integers[0] == v0);
    THES_ASSERT(v == vec[0]);
    vec[0] = v0;
    elem_assert();

    const UInt v2 = v;
    swap(v, integers[2]);
    THES_ASSERT(integers[2] == v2);
    THES_ASSERT(v == vec[2]);
    vec[2] = v2;
    elem_assert();
  }

  std::sort(integers.begin(), integers.end());
  std::sort(vec.begin(), vec.end());
  elem_assert();

  for ([[maybe_unused]] const auto i : thes::range<std::size_t>(4)) {
    pop_back();
    elem_assert();
  }
}
