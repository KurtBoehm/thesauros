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
  {
    using namespace thes::literals;

    static constexpr thes::StaticMap map{"a"_key = 2, "bc"_key = 4};
    using Map = decltype(map);

    static_assert(!Map::only_keys<"a"_sstr>);
    static_assert(Map::only_keys<"a"_sstr, "bc"_sstr>);
    static_assert(Map::only_keys<"a"_sstr, "bc"_sstr, "c"_sstr>);

    static_assert(Map::contains<"a"_sstr>);
    static_assert(!Map::contains<"b"_sstr>);

    static_assert(map.get<"a"_sstr>() == 2);
    static_assert(map.get<"bc"_sstr>() == 4);
  }
}
