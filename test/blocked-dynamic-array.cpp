#include "thesauros/containers.hpp"
#include "thesauros/format.hpp"
#include "thesauros/ranges.hpp"

#include "tools.hpp"

namespace test = thes::test;

struct S {
  S() = default;
  explicit S(int j) : i(j) {}

  S(const S&) = default;
  S(S&&) = default;
  S& operator=(const S&) = default;
  S& operator=(S&&) = default;

  ~S() {
    ++counter();
  }

  int i{2};

  static int& counter() {
    static int ctr{0};
    return ctr;
  }

  friend std::ostream& operator<<(std::ostream& stream, const S& self) {
    return stream << "S(" << self.i << ")";
  }

  bool operator==(const S&) const = default;
};

using Blocked = thes::BlockedDynamicArray<S>;
using Nested = thes::NestedDynamicArray<S>;
using NestedBuilder = typename Nested::NestedBuilder;

int main() {
  Blocked vec(8);
  THES_ASSERT(vec.block_num() == 0 && vec.value_num() == 0);

  vec.push_block();

  {
    auto block1 = *(vec.end() - 1);
    THES_ASSERT(vec.block_num() == 1 && vec.value_num() == 0);
    THES_ASSERT(block1.size() == 0);

    block1.emplace_back(3);
    THES_ASSERT(vec.block_num() == 1 && vec.value_num() == 1);
    THES_ASSERT(test::rangeq(block1, std::array{S{3}}));

    for (const auto i : thes::range<int>(1, 8)) {
      block1.emplace_back(i);
    }
    THES_ASSERT(vec.block_num() == 1 && vec.value_num() == 8);
    THES_ASSERT(test::rangeq(block1, std::array{S{3}, S{1}, S{2}, S{3}, S{4}, S{5}, S{6}, S{7}}));
  }

  {
    S::counter() = 0;
    vec.push_block();
    auto block1 = vec[0];
    auto block2 = *(vec.end() - 1);
    THES_ASSERT(S::counter() == 8);
    THES_ASSERT(vec.block_num() == 2 && vec.value_num() == 8);
    THES_ASSERT(test::rangeq(block1, std::array{S{3}, S{1}, S{2}, S{3}, S{4}, S{5}, S{6}, S{7}}));
    THES_ASSERT(block2.size() == 0);

    block2.emplace_back(2);
    block2.emplace_back(3);
    block2.emplace_back(5);
    block2.emplace_back(11);
    block2.emplace_back(17);
    THES_ASSERT(vec.block_num() == 2 && vec.value_num() == 13);
    THES_ASSERT(test::rangeq(block1, std::array{S{3}, S{1}, S{2}, S{3}, S{4}, S{5}, S{6}, S{7}}));
    THES_ASSERT(test::rangeq(block2, std::array{S{2}, S{3}, S{5}, S{11}, S{17}}));

    block1.erase(S(2));
    block1.erase(S(3));
    THES_ASSERT(vec.block_num() == 2 && vec.value_num() == 10);
    THES_ASSERT(test::rangeq(block1, std::array{S{1}, S{4}, S{5}, S{6}, S{7}}));
    THES_ASSERT(test::rangeq(block2, std::array{S{2}, S{3}, S{5}, S{11}, S{17}}));

    S::counter() = 0;
    for ([[maybe_unused]] const auto i : thes::range<std::size_t>(30)) {
      vec.push_block();
      vec.push_block();
    }
    THES_ASSERT(S::counter() == 40);
  }

  {
    auto block1 = vec[0];
    auto block2 = vec[1];
    THES_ASSERT(vec.block_num() == 62 && vec.value_num() == 10);
    THES_ASSERT(test::rangeq(block1, std::array{S{1}, S{4}, S{5}, S{6}, S{7}}));
    THES_ASSERT(test::rangeq(block2, std::array{S{2}, S{3}, S{5}, S{11}, S{17}}));
  }

  auto builder = NestedBuilder();
  builder.initialize(vec.block_num(), vec.value_num());
  {
    auto part = builder.part_builder(0, 0);
    for (auto&& val : vec[0]) {
      part.emplace(val);
    }
    part.advance_group();
  }
  {
    auto part = builder.part_builder(1, vec[0].size());
    for (const std::size_t i : thes::range<std::size_t>(1, vec.block_num())) {
      for (auto&& val : vec[i]) {
        part.emplace(val);
      }
      part.advance_group();
    }
  }

  Nested nested = builder.build();
  {
    THES_ASSERT(nested.group_num() == 62 && nested.element_num() == 10);
    THES_ASSERT(test::rangeq(vec[0], std::array{S{1}, S{4}, S{5}, S{6}, S{7}}));
    THES_ASSERT(test::rangeq(vec[1], std::array{S{2}, S{3}, S{5}, S{11}, S{17}}));
  }
}
