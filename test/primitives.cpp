#include <cassert>
#include <concepts>

#include "thesauros/utility.hpp"

int main() {
  static_assert(std::same_as<thes::u8, std::uint8_t>);

  static_assert(std::same_as<thes::FixedUnsignedInt<4>, std::uint32_t>);
  static_assert(std::same_as<thes::FixedUnsignedInt<5>, std::uint64_t>);
  static_assert(std::same_as<thes::FixedUnsignedInt<9>, thes::u128>);
  static_assert(std::same_as<thes::FixedSignedInt<9>, thes::i128>);

  static_assert(std::same_as<thes::Union<thes::u8, thes::u16>, thes::u16>);
  static_assert(std::same_as<thes::Union<thes::u32, thes::u16>, thes::u32>);
  static_assert(std::same_as<thes::Intersection<thes::u8, thes::u16>, thes::u8>);
  static_assert(std::same_as<thes::Intersection<thes::u32, thes::u16>, thes::u16>);
}
