#include "thesauros/utility.hpp"

int main() {
  static_assert(!thes::ConstAccess<int>);
  static_assert(thes::ConstAccess<const int>);
  static_assert(!thes::ConstAccess<int&>);
  static_assert(thes::ConstAccess<const int&>);
}
