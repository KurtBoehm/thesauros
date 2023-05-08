#include <array>
#include <iostream>

#include "thesauros/utility.hpp"

int main() {
  using namespace thes::literals;

  static constexpr auto msg1 = "aBc"_sstr;
  static constexpr auto msg2 = "Łabędź żółty"_sstr;
  static_assert(thes::star::IsStaticRange<decltype(msg2)>);
  std::cout << msg2.view() << std::endl;

  static constexpr auto msg_arr_1 = msg1 | thes::star::to_array;
  static_assert(msg_arr_1 == std::array{'a', 'B', 'c'});
  static constexpr auto msg_arr_2 = msg1.to_lowercase() | thes::star::to_array;
  static_assert(msg_arr_2 == std::array{'a', 'b', 'c'});
  static constexpr auto msg_arr_3 = thes::to_snake_case<msg1>() | thes::star::to_array;
  static_assert(msg_arr_3 == std::array{'a', '_', 'b', 'c'});
}
