// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <charconv>
#include <concepts>
#include <iterator>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "thesauros/charconv/numeric-string.hpp"
#include "thesauros/charconv/parse-integer.hpp"
#include "thesauros/charconv/string-convert.hpp"
#include "thesauros/charconv/string-escape.hpp"
#include "thesauros/charconv/unicode.hpp"
#include "thesauros/test/test.hpp"

namespace {
using namespace std::string_view_literals;

//==================================================================================================
// Compile-time evaluation
//==================================================================================================

/** Confirms `numeric_string` round-trips integral values at compile time. */
consteval bool numeric_string_works_at_compile_time() {
  const auto integral = thes::numeric_string(42);
  if (!integral.has_value() || std::string_view(integral->data(), integral->size()) != "42") {
    return false;
  }
  const auto negative = thes::numeric_string(-7);
  return negative.has_value() && std::string_view(negative->data(), negative->size()) == "-7";
}
static_assert(numeric_string_works_at_compile_time());

/** Confirms `parse_integer` parses decimal, hexadecimal, and invalid input at compile time. */
consteval bool parse_integer_works_at_compile_time() {
  const auto decimal = thes::parse_integer<int>("42");
  if (!decimal.has_value() || *decimal != 42) {
    return false;
  }
  const auto hex = thes::parse_integer<int>("0x1A");
  if (!hex.has_value() || *hex != 26) {
    return false;
  }
  return !thes::parse_integer<int>("").has_value();
}
static_assert(parse_integer_works_at_compile_time());

/** Confirms `string_to_integral` parses well-formed and rejects malformed input at compile time. */
consteval bool string_to_integral_works_at_compile_time() {
  const auto value = thes::string_to_integral<int>("42");
  if (!value.has_value() || *value != 42) {
    return false;
  }
  const auto hex = thes::string_to_integral<unsigned>("2a", 16);
  if (!hex.has_value() || *hex != 0x2AU) {
    return false;
  }
  return !thes::string_to_integral<int>("abc").has_value();
}
static_assert(string_to_integral_works_at_compile_time());

//==================================================================================================
// numeric_string
//==================================================================================================

THES_TEMPLATE_TEST_CASE("numeric_string round-trips integral values", "[charconv][numeric_string]",
                        thes::u8, thes::i8, thes::u16, thes::i16, thes::u32, thes::i32, thes::u64,
                        thes::i64) {
  const auto zero = thes::numeric_string(TestType{0});
  THES_REQUIRE(zero.has_value());
  THES_CHECK(std::string_view(zero->data(), zero->size()) == "0");

  const auto positive = thes::numeric_string(TestType{42});
  THES_REQUIRE(positive.has_value());
  THES_CHECK(std::string_view(positive->data(), positive->size()) == "42");

  if constexpr (std::signed_integral<TestType>) {
    const auto negative = thes::numeric_string(TestType{-42});
    THES_REQUIRE(negative.has_value());
    THES_CHECK(std::string_view(negative->data(), negative->size()) == "-42");
  }
}

THES_TEST_CASE("numeric_string round-trips floating-point values", "[charconv][numeric_string]") {
  auto round_trips = [](auto value) {
    using T = decltype(value);
    const auto text = thes::numeric_string(value);
    if (!text.has_value()) {
      return false;
    }
    T parsed{};
    const auto res = std::from_chars(text->data(), text->data() + text->size(), parsed);
    return res.ec == std::errc{} && parsed == value;
  };

  THES_CHECK(round_trips(0.0));
  THES_CHECK(round_trips(3.5));
  THES_CHECK(round_trips(-2.25F));
  THES_CHECK(round_trips(std::numeric_limits<double>::max()));
}

//==================================================================================================
// parse_integer
//==================================================================================================

THES_TEST_CASE_PARAM("parse_integer parses decimal values", "[charconv][parse_integer]", int, value,
                     0, 1, 42, 12345, -1, -42) {
  const auto text = thes::numeric_string(value);
  THES_REQUIRE(text.has_value());
  const auto parsed = thes::parse_integer<int>(std::string_view{text->data(), text->size()});
  THES_REQUIRE(parsed.has_value());
  THES_CHECK(*parsed == value);
}

THES_TEST_CASE("parse_integer parses hexadecimal and binary literals",
               "[charconv][parse_integer]") {
  THES_CHECK(thes::parse_integer<int>("0x1A") == 26);
  THES_CHECK(thes::parse_integer<int>("0X1a") == 26);
  THES_CHECK(thes::parse_integer<int>("0b101") == 5);
  THES_CHECK(thes::parse_integer<int>("0B101") == 5);
  THES_CHECK(thes::parse_integer<int>("0") == 0);
}

THES_TEST_CASE("parse_integer skips digit separators in extended mode",
               "[charconv][parse_integer]") {
  THES_CHECK(thes::parse_integer<int>("1_000_000") == 1000000);
  THES_CHECK(thes::parse_integer<int>("1'000'000") == 1000000);
}

THES_TEST_CASE("parse_integer literal mode treats a leading zero as octal",
               "[charconv][parse_integer]") {
  using enum thes::IntegerParseMode;
  const auto parsed = thes::parse_integer<int, thes::AutoTag<literal>>("017");
  THES_REQUIRE(parsed.has_value());
  THES_CHECK(*parsed == 15);
}

THES_TEST_CASE("parse_integer returns an empty result for empty input",
               "[charconv][parse_integer]") {
  THES_CHECK(!thes::parse_integer<int>("").has_value());
  THES_CHECK(!thes::parse_integer<int>("-").has_value());
}

THES_TEST_CASE("parse_integer throws on an unrecognized digit character",
               "[charconv][parse_integer]") {
  THES_CHECK_THROWS_AS(thes::parse_integer<int>("12a3"), std::bad_optional_access);
}

//==================================================================================================
// string_to_integral
//==================================================================================================

THES_TEST_CASE("string_to_integral parses well-formed input", "[charconv][string_to_integral]") {
  THES_CHECK(thes::string_to_integral<int>("42"sv) == 42);
  THES_CHECK(thes::string_to_integral<int>("-42"sv) == -42);
  THES_CHECK(thes::string_to_integral<unsigned>("2a"sv, 16) == 0x2AU);
}

THES_TEST_CASE("string_to_integral rejects malformed or partial input",
               "[charconv][string_to_integral]") {
  THES_CHECK(!thes::string_to_integral<int>(""sv).has_value());
  THES_CHECK(!thes::string_to_integral<int>("abc"sv).has_value());
  THES_CHECK(!thes::string_to_integral<int>("42abc"sv).has_value());
  THES_CHECK(!thes::string_to_integral<int>("999999999999999999999"sv).has_value());
}

//==================================================================================================
// escape_string
//==================================================================================================

THES_TEST_CASE("escape_string leaves printable ASCII untouched", "[charconv][escape_string]") {
  std::string out{};
  thes::escape_string("Hello, World!", std::back_inserter(out));
  THES_CHECK(out == "Hello, World!");
}

THES_TEST_CASE("escape_string escapes control characters and quotes", "[charconv][escape_string]") {
  std::string out{};
  thes::escape_string("a\tb\nc\"d\\e"sv, std::back_inserter(out));
  THES_CHECK(out == "a\\tb\\nc\\\"d\\\\e");
}

THES_TEST_CASE("escape_string escapes other control characters as unicode escapes",
               "[charconv][escape_string]") {
  std::string out{};
  thes::escape_string("\x01\x1F"sv, std::back_inserter(out));
  THES_CHECK(out == "\\u0001\\u001F");
}

THES_TEST_CASE("escape_string preserves valid multi-byte UTF-8", "[charconv][escape_string]") {
  std::string out{};
  const std::string_view input = "caf\xC3\xA9"sv;
  thes::escape_string(input, std::back_inserter(out));
  THES_CHECK(out == input);
}

THES_TEST_CASE("escape_string throws on an invalid byte", "[charconv][escape_string]") {
  std::string out{};
  const std::string_view input = "\xFF"sv;
  THES_CHECK_THROWS_AS(thes::escape_string(input, std::back_inserter(out)), std::invalid_argument);
}

THES_TEST_CASE("escape_string throws on a truncated multi-byte sequence",
               "[charconv][escape_string]") {
  std::string out{};
  const std::string_view input = "\xC2"sv;
  THES_CHECK_THROWS_AS(thes::escape_string(input, std::back_inserter(out)), std::invalid_argument);
}

//==================================================================================================
// UnicodeDecoder
//==================================================================================================

THES_TEST_CASE("UnicodeDecoder decodes single-byte ASCII", "[charconv][unicode]") {
  thes::UnicodeDecoder decoder{};
  const auto [codep, state] = decoder.decode(thes::u8{'A'});
  THES_CHECK(state == thes::UnicodeDecoder::State::ACCEPTED);
  THES_CHECK(codep == thes::u32{'A'});
}

THES_TEST_CASE("UnicodeDecoder decodes a multi-byte codepoint", "[charconv][unicode]") {
  thes::UnicodeDecoder decoder{};
  const auto [codep, rest] = decoder.decode("\xC3\xA9xyz"sv);
  THES_CHECK(codep == 0xE9U);
  THES_CHECK(rest == "xyz");
}

THES_TEST_CASE("UnicodeDecoder throws on an invalid byte", "[charconv][unicode]") {
  thes::UnicodeDecoder decoder{};
  THES_CHECK_THROWS_AS(decoder.decode("\xFF"sv), std::invalid_argument);
}

THES_TEST_CASE("UnicodeDecoder throws on a truncated sequence", "[charconv][unicode]") {
  thes::UnicodeDecoder decoder{};
  THES_CHECK_THROWS_AS(decoder.decode("\xC2"sv), std::invalid_argument);
}
} // namespace

THES_TEST_MAIN()
