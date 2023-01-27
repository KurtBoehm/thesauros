#include <cassert>
#include <concepts>

#include "thesauros/utility.hpp"

int main() {
  static_assert(std::same_as<thes::U8, std::uint8_t>);

  static_assert(std::same_as<thes::FixedUnsignedInt<4>, std::uint32_t>);
  static_assert(std::same_as<thes::FixedUnsignedInt<5>, std::uint64_t>);
  static_assert(std::same_as<thes::FixedUnsignedInt<9>, thes::U128>);
  static_assert(std::same_as<thes::FixedSignedInt<9>, thes::I128>);

  static_assert(std::same_as<thes::Union<thes::U8, thes::U16>, thes::U16>);
  static_assert(std::same_as<thes::Union<thes::U32, thes::U16>, thes::U32>);
}
