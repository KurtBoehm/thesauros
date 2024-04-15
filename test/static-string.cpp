#include <array>

#include "thesauros/format.hpp"
#include "thesauros/utility.hpp"

int main() {
  using namespace thes::literals;

  static constexpr auto msg1 = "aBc"_sstr;
  static constexpr auto msg2 = "Łabędź żółty"_sstr;
  static_assert(thes::star::AnyStaticRange<decltype(msg2)>);
  fmt::print("{}\n", msg2);

  static constexpr auto msg_arr_1 = msg1 | thes::star::to_array;
  static_assert(msg_arr_1 == std::array{'a', 'B', 'c'});
  static constexpr auto msg_arr_2 = msg1.to_lowercase() | thes::star::to_array;
  static_assert(msg_arr_2 == std::array{'a', 'b', 'c'});
  static constexpr auto msg_arr_3 = thes::to_snake_case<msg1>() | thes::star::to_array;
  static_assert(msg_arr_3 == std::array{'a', '_', 'b', 'c'});

  static constexpr auto joined0 = "_"_sstr.join();
  static_assert((joined0 | thes::star::to_array) == std::array<char, 0>{});
  static constexpr auto joined = "_"_sstr.join("aa"_sstr, "bb"_sstr);
  static_assert((joined | thes::star::to_array) == std::array{'a', 'a', '_', 'b', 'b'});
}
