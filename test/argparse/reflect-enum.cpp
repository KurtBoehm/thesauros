// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <array>
#include <cstdio>
#include <optional>
#include <string>
#include <string_view>

#include "thesauros/argparse.hpp"
#include "thesauros/reflection/enum.hpp"
#include "thesauros/test/test.hpp"
#include "thesauros/types/primitives.hpp"

// Declared outside the anonymous namespace so that the reflection hook `THES_DEFINE_ENUM` defines
// keeps external linkage, as it would in the program the enumeration belongs to.
namespace inner {
THES_DEFINE_ENUM(SNAKE_CASE(Compression), thes::u8, LOWERCASE(None), LOWERCASE(Fast),
                 SNAKE_CASE(HighRatio))
} // namespace inner

namespace {
namespace ap = thes::argparse;
using inner::Compression;

//==================================================================================================
// Conversion
//==================================================================================================

THES_TEST_CASE("A reflected enumeration is an argument value type", "[argparse][reflect]") {
  static_assert(ap::ArgumentValue<Compression>);
  THES_CHECK(ap::parse_value<Compression>("none") == Compression::None);
  THES_CHECK(ap::parse_value<Compression>("fast") == Compression::Fast);
  // The serial name is the one reflection derives, not the name of the enumerator.
  THES_CHECK(ap::parse_value<Compression>("high_ratio") == Compression::HighRatio);
  THES_CHECK(!ap::parse_value<Compression>("HighRatio").has_value());
  THES_CHECK(!ap::parse_value<Compression>("bogus").has_value());
  THES_CHECK(!ap::parse_value<Compression>("").has_value());
}

//==================================================================================================
// Parsing
//==================================================================================================

inline constexpr ap::ArgumentParser parser{
  ap::ProgramInfo{.name = "packer"},
  ap::positional<"input">().help("The file to pack"),
  ap::option<"compression", Compression>("-c")
    .default_to(Compression::Fast)
    .help("How hard to squeeze"),
  ap::option<"fallback", Compression>("-f")
    .choices(Compression::None, Compression::Fast)
    .help("What to fall back to"),
};

/** Confirms that an enumerated value is converted at compile time like any other. */
consteval bool parses_at_compile_time() {
  constexpr std::array tokens{"in.txt", "--compression=high_ratio"};
  const auto result = parser.parse(tokens);
  return result.has_value() && result->get<"compression">() == Compression::HighRatio;
}
static_assert(parses_at_compile_time());

THES_TEST_CASE("An enumerated option is parsed and defaulted", "[argparse][reflect]") {
  constexpr std::array tokens{"in.txt"};
  const auto result = parser.parse(tokens);
  THES_REQUIRE(result.has_value());
  THES_CHECK(result->get<"compression">() == Compression::Fast);
  THES_CHECK(!result->get<"fallback">().has_value());

  constexpr std::array given{"in.txt", "-c", "none"};
  const auto chosen = parser.parse(given);
  THES_REQUIRE(chosen.has_value());
  THES_CHECK(chosen->get<"compression">() == Compression::None);
}

THES_TEST_CASE("An unknown enumerator is an invalid value", "[argparse][reflect]") {
  constexpr std::array tokens{"in.txt", "-c", "bogus"};
  const auto result = parser.parse(tokens);
  THES_REQUIRE(!result.has_value());
  THES_CHECK(result.error().kind == ap::ParseErrorKind::invalid_value);
  THES_CHECK(result.error().argument == "--compression");
  THES_CHECK(result.error().value == "bogus");
}

THES_TEST_CASE("Choices narrow an enumeration further", "[argparse][reflect]") {
  constexpr std::array tokens{"in.txt", "-f", "high_ratio"};
  const auto result = parser.parse(tokens);
  THES_REQUIRE(!result.has_value());
  THES_CHECK(result.error().kind == ap::ParseErrorKind::invalid_choice);
}

//==================================================================================================
// Help output
//==================================================================================================

/** The help text `help_parser` writes, captured through a temporary file. */
std::string capture_help(const auto& help_parser) {
  std::FILE* file = std::tmpfile();
  if (file == nullptr) {
    return {};
  }
  help_parser.print_help(file);
  std::rewind(file); // NOLINT

  std::string text{};
  std::array<char, 256> buffer{};
  while (const std::size_t count = std::fread(buffer.data(), 1, buffer.size(), file)) {
    text.append(buffer.data(), count);
  }
  static_cast<void>(std::fclose(file));
  return text;
}

THES_TEST_CASE("The help text names the enumerators", "[argparse][reflect]") {
  const std::string help = capture_help(parser);
  // Without choices of its own, an enumerated argument lists every enumerator.
  THES_CHECK(help.find("(choices: none, fast, high_ratio)") != std::string::npos);
  // With choices, it lists those.
  THES_CHECK(help.find("(choices: none, fast)") != std::string::npos);
  // The default is shown by its serial name rather than as a number.
  THES_CHECK(help.find("(default: fast)") != std::string::npos);
}
} // namespace

THES_TEST_MAIN()
