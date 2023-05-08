#include "thesauros/utility/empty.hpp"

int main() {
  static constexpr thes::Empty empty{};
  static_assert(!empty.has_value());

  static_assert(thes::apply_empty([](int i) { return i; }, std::make_tuple(1)) == 1);
  static_assert(thes::apply_empty([](int /*i*/) { return; }, std::make_tuple(1)) == empty);
}
