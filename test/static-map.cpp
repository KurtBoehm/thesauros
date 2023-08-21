#include "thesauros/utility.hpp"

int main() {
  {
    static constexpr thes::StaticMap map{};
    using Map = decltype(map);
    static_assert(!Map::contains<2>);
  }
  {
    static constexpr thes::StaticMap map{thes::static_key<4> = 3};
    using Map = decltype(map);
    static_assert(!Map::contains<2>);
    static_assert(Map::contains<4>);
    static_assert(map.get<4>() == 3);
  }
  {
    static constexpr thes::StaticMap map{thes::static_key<4> = 3, thes::static_key<3> = 2};
    using Map = decltype(map);
    static_assert(!Map::contains<2>);
    static_assert(Map::contains<4>);
    static_assert(map.get<3>() == 2);
    static_assert(map.get<4>() == 3);
  }
}
