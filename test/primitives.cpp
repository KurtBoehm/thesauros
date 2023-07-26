#include <concepts>
#include <cstdint>

#include "thesauros/utility.hpp"

int main() {
  using namespace thes::literals;

  static_assert(std::same_as<thes::u8, std::uint8_t>);

  static_assert(std::same_as<thes::FixedUnsignedInt<4>, std::uint32_t>);
  static_assert(std::same_as<thes::FixedUnsignedInt<5>, std::uint64_t>);
  static_assert(std::same_as<thes::FixedUnsignedInt<9>, thes::u128>);
  static_assert(std::same_as<thes::FixedSignedInt<9>, thes::i128>);

  static_assert(std::same_as<thes::Union<thes::u8, thes::u16>, thes::u16>);
  static_assert(std::same_as<thes::Union<thes::u32, thes::u16>, thes::u32>);
  static_assert(std::same_as<thes::Union<thes::u8, thes::u32, thes::u16>, thes::u32>);
  static_assert(std::same_as<thes::Union<thes::f64, thes::f32>, thes::f64>);
  static_assert(std::same_as<thes::Intersection<thes::u8, thes::u16>, thes::u8>);
  static_assert(std::same_as<thes::Intersection<thes::u32, thes::u16>, thes::u16>);
  static_assert(std::same_as<thes::Intersection<thes::u8, thes::u32, thes::u16>, thes::u8>);
  static_assert(std::same_as<thes::Intersection<thes::f32, thes::f64>, thes::f32>);

  static_assert(123_u8 == 123);
  static_assert("0x23"_u8 == 0x23);
  static_assert("0x123"_u16 == 0x123);
  static_assert("0b101"_u16 == 0b101);

  static_assert("-0b1'000'000"_iz == -0b1'000'000);
  static_assert(-"0b1000000"_iz == -0b1'000'000);
  static_assert(
    "-0b1000000000000000000000000000000000000000000000000000000000000000"_iz ==
    static_cast<long>(-0b1000000000000000000000000000000000000000000000000000000000000000));
  static_assert(-"0b1000000000000000000000000000000000000000000000000000000000000000"_uz ==
                -0b1000000000000000000000000000000000000000000000000000000000000000UL);
  static_assert("128"_u8 == 128);
  static_assert(32_f32 == 32);
}
