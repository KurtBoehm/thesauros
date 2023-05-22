#include <algorithm>
#include <utility>
#include <vector>

#include "thesauros/containers.hpp"
#include "thesauros/io.hpp"
#include "thesauros/test.hpp"

namespace test = thes::test;

int main() {
  thes::StronglyOrderedMap<int, int> map{};
  std::vector<thes::strong_order::Pair<int, int>> ref{};

  auto assert_eq = [&] {
    std::cout << thes::range_print(map) << '\n';
    THES_ASSERT(test::range_eq(map, ref));
  };

  auto insert = [&](int key, int value) {
    const auto res1 = map.insert(key, value);

    auto it = std::find_if(ref.begin(), ref.end(), [&](auto e) { return e.key() == key; });
    const auto res2 = it == ref.end();
    if (res2) {
      ref.emplace_back(key, value);
      std::sort(ref.begin(), ref.end());
    }

    THES_ASSERT(res1 == res2);
    std::cout << key << ' ' << (res1 ? "inserted" : "not inserted") << '\n';
    assert_eq();
  };

  auto erase = [&](int key) {
    const auto res1 = map.erase(key);

    auto it = std::find_if(ref.begin(), ref.end(), [&](auto e) { return e.key() == key; });
    const auto res2 = it != ref.end();
    if (res2) {
      ref.erase(it);
    }

    THES_ASSERT(res1 == res2);
    std::cout << key << ' ' << (res1 ? "found" : "not found") << '\n';
    assert_eq();
  };

  assert_eq();
  insert(16, 32);
  insert(4, 16);

  erase(8);
  erase(4);

  insert(4, 8);
  insert(4, 16);
}
