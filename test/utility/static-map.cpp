// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "thesauros/math.hpp"
#include "thesauros/string.hpp"
#include "thesauros/types.hpp"
#include "thesauros/utility.hpp"

int main() {
  {
    static constexpr thes::StaticMap map{};
    using Map = decltype(map);
    static_assert(!Map::contains(thes::auto_tag<2>));
  }
  {
    static constexpr thes::StaticMap map{thes::static_key<4> = 3};
    using Map = decltype(map);
    static_assert(!Map::contains(thes::auto_tag<2>));
    static_assert(Map::contains(thes::auto_tag<4>));
    static_assert(map.get(thes::auto_tag<4>) == 3);
  }
  {
    static constexpr thes::StaticMap map{thes::static_key<4> = 3, thes::static_key<3> = 2};
    using Map = decltype(map);
    static_assert(!Map::contains(thes::auto_tag<2>));
    static_assert(Map::contains(thes::auto_tag<4>));
    static_assert(map.get(thes::auto_tag<3>) == 2);
    static_assert(map.get(thes::auto_tag<4>) == 3);
  }
  {
    using namespace thes::literals;

    static constexpr thes::StaticMap map{"a"_key = 2, "bc"_key = 4};
    using Map = decltype(map);

    static_assert(!Map::only_keys<"a"_sstr>);
    static_assert(Map::only_keys<"a"_sstr, "bc"_sstr>);
    static_assert(Map::only_keys<"a"_sstr, "bc"_sstr, "c"_sstr>);

    static_assert(Map::contains(thes::auto_tag<"a"_sstr>));
    static_assert(!Map::contains(thes::auto_tag<"b"_sstr>));

    static_assert(map.get(thes::auto_tag<"a"_sstr>) == 2);
    static_assert(map.get(thes::auto_tag<"bc"_sstr>) == 4);
  }
  {
    using namespace thes::literals;

    static constexpr float a = 0.1F;
    static constexpr float b = 0.101F;

    static_assert(!thes::is_close(a, b));
    static_assert(!thes::is_close(a, b, "rel_tol"_key = 1e-3F));
    static_assert(!thes::is_close(a, b, "abs_tol"_key = 1e-3F));
    static_assert(thes::is_close(a, b, "rel_tol"_key = 1e-2F));
    static_assert(thes::is_close(a, b, "abs_tol"_key = 1e-2F));
    static_assert(thes::is_close(a, b, "rel_tol"_key = 1e-3F, "abs_tol"_key = 1e-2F));
  }
}
