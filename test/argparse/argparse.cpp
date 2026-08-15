// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdio>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include "thesauros/argparse.hpp"
#include "thesauros/test/test.hpp"
#include "thesauros/types/primitives.hpp"
#include "thesauros/types/type-tag.hpp"

namespace {
namespace ap = thes::argparse;
using namespace std::string_view_literals;

/** What `write` writes to the file it is handed, captured through a temporary file. */
std::string capture(const auto& write) {
  std::FILE* file = std::tmpfile();
  if (file == nullptr) {
    return {};
  }
  write(file);
  std::rewind(file); // NOLINT

  std::string text{};
  std::array<char, 256> buffer{};
  while (const std::size_t count = std::fread(buffer.data(), 1, buffer.size(), file)) {
    text.append(buffer.data(), count);
  }
  static_cast<void>(std::fclose(file));
  return text;
}

/** The help text `help_parser` writes. */
std::string capture_help(const auto& help_parser) {
  return capture([&](std::FILE* file) { help_parser.print_help(file); });
}

//==================================================================================================
// Value conversion
//==================================================================================================

/** A type showing that a custom `parse_argument_value` is found by argument-dependent lookup. */
struct Fraction {
  int numerator{};
  int denominator{1};

  friend constexpr bool operator==(const Fraction&, const Fraction&) = default; // NOLINT
};

constexpr std::optional<Fraction> parse_argument_value(thes::TypeTag<Fraction> /*tag*/,
                                                       std::string_view text) {
  const std::size_t slash = text.find('/');
  if (slash == std::string_view::npos) {
    return std::nullopt;
  }
  const auto numerator = ap::parse_value<int>(text.substr(0, slash));
  const auto denominator = ap::parse_value<int>(text.substr(slash + 1));
  if (!numerator.has_value() || !denominator.has_value() || *denominator == 0) {
    return std::nullopt;
  }
  return Fraction{.numerator = *numerator, .denominator = *denominator};
}

THES_TEST_CASE("parse_integer_value reads the supported bases", "[argparse][value]") {
  THES_CHECK(ap::parse_integer_value<int>("42") == 42);
  THES_CHECK(ap::parse_integer_value<int>("+42") == 42);
  THES_CHECK(ap::parse_integer_value<int>("-42") == -42);
  THES_CHECK(ap::parse_integer_value<int>("0x1A") == 26);
  THES_CHECK(ap::parse_integer_value<int>("0X1a") == 26);
  THES_CHECK(ap::parse_integer_value<int>("0b1011") == 11);
  THES_CHECK(ap::parse_integer_value<int>("0o17") == 15);
  THES_CHECK(ap::parse_integer_value<int>("1_000_000") == 1000000);
  THES_CHECK(ap::parse_integer_value<int>("1'000") == 1000);
  // A leading zero is no octal prefix, unlike in a C++ literal.
  THES_CHECK(ap::parse_integer_value<int>("017") == 17);
}

THES_TEST_CASE("parse_integer_value rejects malformed input", "[argparse][value]") {
  THES_CHECK(!ap::parse_integer_value<int>("").has_value());
  THES_CHECK(!ap::parse_integer_value<int>("-").has_value());
  THES_CHECK(!ap::parse_integer_value<int>("12a").has_value());
  THES_CHECK(!ap::parse_integer_value<int>("0b12").has_value());
  THES_CHECK(!ap::parse_integer_value<int>("0x").has_value());
  THES_CHECK(!ap::parse_integer_value<int>("_1").has_value());
  THES_CHECK(!ap::parse_integer_value<int>("1_").has_value());
  THES_CHECK(!ap::parse_integer_value<unsigned>("-1").has_value());
}

THES_TEST_CASE("parse_integer_value respects the range of its type", "[argparse][value]") {
  THES_CHECK(ap::parse_integer_value<thes::i8>("127") == thes::i8{127});
  THES_CHECK(ap::parse_integer_value<thes::i8>("-128") == thes::i8{-128});
  THES_CHECK(!ap::parse_integer_value<thes::i8>("128").has_value());
  THES_CHECK(!ap::parse_integer_value<thes::i8>("-129").has_value());
  THES_CHECK(ap::parse_integer_value<thes::u8>("255") == thes::u8{255});
  THES_CHECK(!ap::parse_integer_value<thes::u8>("256").has_value());
  THES_CHECK(ap::parse_integer_value<thes::u64>("18446744073709551615") ==
             thes::u64{18446744073709551615U});
  THES_CHECK(!ap::parse_integer_value<thes::u64>("18446744073709551616").has_value());
}

THES_TEST_CASE("parse_value handles the built-in types", "[argparse][value]") {
  THES_CHECK(ap::parse_value<std::string_view>("text") == "text");
  THES_CHECK(ap::parse_value<bool>("TRUE") == true);
  THES_CHECK(ap::parse_value<bool>("off") == false);
  THES_CHECK(!ap::parse_value<bool>("maybe").has_value());
  THES_CHECK(ap::parse_value<double>("1.5") == 1.5);
  THES_CHECK(!ap::parse_value<double>("1.5x").has_value());
}

THES_TEST_CASE("parse_value prefers a custom conversion", "[argparse][value]") {
  constexpr Fraction three_quarters{.numerator = 3, .denominator = 4};
  THES_CHECK(ap::parse_value<Fraction>("3/4") == three_quarters);
  THES_CHECK(!ap::parse_value<Fraction>("3").has_value());
  THES_CHECK(!ap::parse_value<Fraction>("3/0").has_value());
}

//==================================================================================================
// Parsing
//==================================================================================================

inline constexpr ap::ArgumentParser parser{
  ap::ProgramInfo{
    .name = "demo",
    .version = "1.2.3",
    .description = "A parser exercising every kind of argument.",
    .epilog = "The epilog.",
  },
  ap::positional<"input">().help("The file to read"),
  ap::positional<"output">().default_to("out.txt"sv).help("The file to write"),
  ap::option<"count", int>("-c").default_to(1).help("How often to read the input"),
  ap::option<"scale", double>("-s").help("An optional scale factor"),
  ap::option<"mode">("-m").choices("fast"sv, "slow"sv).default_to("fast"sv).help("The mode"),
  ap::flag<"dry-run">("-n").help("Do not write anything"),
  ap::counter<"verbose">("-v").help("Increase verbosity"),
  ap::remainder<"rest">().help("Arguments passed on verbatim"),
};

/** Parses `tokens` as if they followed the program name on the command line. */
constexpr auto parse(std::span<const char* const> tokens) {
  return parser.parse(tokens);
}

/** Confirms that a whole parse, including conversion, is a constant expression. */
consteval bool parses_at_compile_time() {
  constexpr std::array tokens{"-c", "0x10", "-vvn", "in.txt", "--mode=slow"};
  const auto result = parse(tokens);
  return result.has_value() && result->get<"count">() == 16 && result->get<"verbose">() == 2U &&
         result->get<"dry-run">() && result->get<"input">() == "in.txt" &&
         result->get<"output">() == "out.txt" && result->get<"mode">() == "slow" &&
         !result->get<"scale">().has_value() && result->get<"rest">().empty();
}
static_assert(parses_at_compile_time());

THES_TEST_CASE("Absent arguments fall back to their defaults", "[argparse][parse]") {
  constexpr std::array tokens{"in.txt"};
  const auto result = parse(tokens);
  THES_REQUIRE(result.has_value());
  THES_CHECK(result->get<"input">() == "in.txt");
  THES_CHECK(result->get<"output">() == "out.txt");
  THES_CHECK(result->get<"count">() == 1);
  THES_CHECK(result->get<"mode">() == "fast");
  THES_CHECK(!result->get<"dry-run">());
  THES_CHECK(result->get<"verbose">() == 0U);
  THES_CHECK(!result->get<"scale">().has_value());
  THES_CHECK(result->get<"rest">().empty());
}

/** The parsed `count` for `tokens`, or `std::nullopt` if they do not parse. */
constexpr std::optional<int> parsed_count(std::span<const char* const> tokens) {
  const auto result = parse(tokens);
  if (!result.has_value()) {
    return std::nullopt;
  }
  return result->get<"count">();
}

THES_TEST_CASE("An option takes its value in every spelling", "[argparse][parse]") {
  THES_CHECK(parsed_count(std::array{"in.txt", "--count", "7"}) == 7);
  THES_CHECK(parsed_count(std::array{"in.txt", "--count=7"}) == 7);
  THES_CHECK(parsed_count(std::array{"in.txt", "-c", "7"}) == 7);
  THES_CHECK(parsed_count(std::array{"in.txt", "-c7"}) == 7);
  THES_CHECK(parsed_count(std::array{"in.txt", "-c=7"}) == 7);
}

THES_TEST_CASE("Short flags are bundled and counted", "[argparse][parse]") {
  constexpr std::array tokens{"-vnv", "in.txt"};
  const auto result = parse(tokens);
  THES_REQUIRE(result.has_value());
  THES_CHECK(result->get<"verbose">() == 2U);
  THES_CHECK(result->get<"dry-run">());
}

THES_TEST_CASE("A bundle may end in an option with a value", "[argparse][parse]") {
  constexpr std::array tokens{"-nc9", "in.txt"};
  const auto result = parse(tokens);
  THES_REQUIRE(result.has_value());
  THES_CHECK(result->get<"dry-run">());
  THES_CHECK(result->get<"count">() == 9);
}

THES_TEST_CASE("Positional arguments are filled in declaration order", "[argparse][parse]") {
  constexpr std::array tokens{"in.txt", "written.txt", "a", "b"};
  const auto result = parse(tokens);
  THES_REQUIRE(result.has_value());
  THES_CHECK(result->get<"input">() == "in.txt");
  THES_CHECK(result->get<"output">() == "written.txt");
  THES_REQUIRE(result->get<"rest">().size() == 2);
  THES_CHECK(std::string_view{result->get<"rest">()[0]} == "a");
  THES_CHECK(std::string_view{result->get<"rest">()[1]} == "b");
}

THES_TEST_CASE("A remainder collects tokens that look like names", "[argparse][parse]") {
  // The first token to reach the remainder starts it; everything after is collected verbatim.
  constexpr std::array tokens{"in.txt", "out.txt", "cmd", "--unknown", "-x"};
  const auto result = parse(tokens);
  THES_REQUIRE(result.has_value());
  THES_REQUIRE(result->get<"rest">().size() == 3);
  THES_CHECK(std::string_view{result->get<"rest">()[1]} == "--unknown");
  THES_CHECK(std::string_view{result->get<"rest">()[2]} == "-x");
}

//--------------------------------------------------------------------------------------------------
// A wrapper handing the rest of the command line on, the way `sudo` does
//--------------------------------------------------------------------------------------------------

inline constexpr ap::ArgumentParser wrapper{
  ap::ProgramInfo{.name = "wrap"},
  ap::flag<"quiet">("-q").help("Say nothing"),
  ap::remainder<"command">().required().help("The command to run"),
};

THES_TEST_CASE("A wrapper reads its own arguments first", "[argparse][parse]") {
  constexpr std::array tokens{"-q", "ls", "-la", "--colour"};
  const auto result = wrapper.parse(tokens);
  THES_REQUIRE(result.has_value());
  THES_CHECK(result->get<"quiet">());
  THES_REQUIRE(result->get<"command">().size() == 3);
  THES_CHECK(std::string_view{result->get<"command">()[0]} == "ls");
  THES_CHECK(std::string_view{result->get<"command">()[2]} == "--colour");
}

THES_TEST_CASE("A remainder may start with a double dash", "[argparse][parse]") {
  constexpr std::array tokens{"--", "-q", "ls"};
  const auto result = wrapper.parse(tokens);
  THES_REQUIRE(result.has_value());
  // `-q` fell behind the separator, so it belongs to the command rather than to the wrapper.
  THES_CHECK(!result->get<"quiet">());
  THES_REQUIRE(result->get<"command">().size() == 2);
  THES_CHECK(std::string_view{result->get<"command">()[0]} == "-q");
}

THES_TEST_CASE("A required remainder demands at least one token", "[argparse][parse]") {
  const auto result = wrapper.parse(std::array{"-q"});
  THES_REQUIRE(!result.has_value());
  THES_CHECK(result.error().kind == ap::ParseErrorKind::missing_argument);
  THES_CHECK(result.error().argument == "COMMAND");
}

//--------------------------------------------------------------------------------------------------
// A list, which is a positional argument’s multiplicity rather than a halt to interpretation
//--------------------------------------------------------------------------------------------------

inline constexpr ap::ArgumentParser collecting{
  ap::ProgramInfo{.name = "downsample"},
  ap::option<"output">("-o").default_to("out"sv).help("Where to write the results"),
  ap::list<"input-files">().required().help("The files to be downsampled"),
};

THES_TEST_CASE("A list leaves the named arguments alone", "[argparse][parse]") {
  // The tokens outlive the results, which reference them.
  constexpr std::array leading{"-o", "dir", "a", "b"};
  constexpr std::array trailing{"a", "b", "-o", "dir"};
  for (const auto& result : {collecting.parse(leading), collecting.parse(trailing)}) {
    THES_REQUIRE(result.has_value());
    THES_CHECK(result->get<"output">() == "dir");
    THES_REQUIRE(result->get<"input-files">().size() == 2);
    THES_CHECK(std::string_view{result->get<"input-files">()[0]} == "a");
    THES_CHECK(std::string_view{result->get<"input-files">()[1]} == "b");
  }
}

THES_TEST_CASE("A list split by a named argument is reported", "[argparse][parse]") {
  const auto result = collecting.parse(std::array{"a", "-o", "dir", "b"});
  THES_REQUIRE(!result.has_value());
  THES_CHECK(result.error().kind == ap::ParseErrorKind::split_values);
  THES_CHECK(result.error().argument == "INPUT-FILES");
  THES_CHECK(result.error().value == "b");
}

//--------------------------------------------------------------------------------------------------
// A list of a fixed or bounded multiplicity
//--------------------------------------------------------------------------------------------------

// A list with an upper bound stops collecting once it is full, so it need not be the last one.
inline constexpr ap::ArgumentParser cropping{
  ap::ProgramInfo{.name = "crop"},
  ap::flag<"verbose">("-v"),
  ap::list<"corner">().exactly(2).help("The x and y of the corner"),
  ap::positional<"output">().help("Where to write"),
};

THES_TEST_CASE("A fixed count collects exactly that many values", "[argparse][parse]") {
  constexpr std::array tokens{"-v", "1", "2", "out.png"};
  const auto result = cropping.parse(tokens);
  THES_REQUIRE(result.has_value());
  THES_CHECK(result->get<"verbose">());
  THES_REQUIRE(result->get<"corner">().size() == 2);
  THES_CHECK(std::string_view{result->get<"corner">()[0]} == "1");
  THES_CHECK(std::string_view{result->get<"corner">()[1]} == "2");
  // The list stopped at its bound, so the next value went to the positional argument behind it.
  THES_CHECK(result->get<"output">() == "out.png");
}

THES_TEST_CASE("A fixed count rejects a wrong number of values", "[argparse][parse]") {
  const auto few = cropping.parse(std::array{"1", "2"});
  THES_REQUIRE(!few.has_value());
  THES_CHECK(few.error().kind == ap::ParseErrorKind::missing_argument);
  THES_CHECK(few.error().argument == "OUTPUT");

  const auto many = cropping.parse(std::array{"1", "2", "3", "out.png"});
  THES_REQUIRE(!many.has_value());
  THES_CHECK(many.error().kind == ap::ParseErrorKind::excess_positional);
}

THES_TEST_CASE("A bounded count admits a range of sizes", "[argparse][parse]") {
  constexpr ap::ArgumentParser ranged{
    ap::ProgramInfo{.name = "resize"},
    ap::list<"sizes">().between(2, 4).help("Two to four sizes"),
  };
  THES_CHECK(!ranged.parse(std::array{"1"}).has_value());
  THES_CHECK(ranged.parse(std::array{"1", "2"}).has_value());
  THES_CHECK(ranged.parse(std::array{"1", "2", "3", "4"}).has_value());
  THES_CHECK(!ranged.parse(std::array{"1", "2", "3", "4", "5"}).has_value());
}

// Every list keeps a range of its own, so a bounded one may be followed by another.
inline constexpr ap::ArgumentParser boxing{
  ap::ProgramInfo{.name = "box"},
  ap::list<"corner">().exactly(2).help("The x and y of the corner"),
  ap::list<"size">().between(1, 3).required().help("One to three extents"),
};

THES_TEST_CASE("Several lists collect side by side", "[argparse][parse]") {
  constexpr std::array tokens{"1", "2", "3"};
  const auto result = boxing.parse(tokens);
  THES_REQUIRE(result.has_value());
  THES_REQUIRE(result->get<"corner">().size() == 2);
  THES_CHECK(std::string_view{result->get<"corner">()[0]} == "1");
  THES_CHECK(std::string_view{result->get<"corner">()[1]} == "2");
  THES_REQUIRE(result->get<"size">().size() == 1);
  THES_CHECK(std::string_view{result->get<"size">()[0]} == "3");
}

THES_TEST_CASE("Each list is measured against its own bounds", "[argparse][parse]") {
  // The first list is short, which the second one having values of its own does not make up for.
  const auto few = boxing.parse(std::array{"1", "2"});
  THES_REQUIRE(!few.has_value());
  THES_CHECK(few.error().kind == ap::ParseErrorKind::missing_argument);
  THES_CHECK(few.error().argument == "SIZE");

  // The second list stops at its upper bound rather than swallowing what follows.
  const auto many = boxing.parse(std::array{"1", "2", "3", "4", "5", "6"});
  THES_REQUIRE(!many.has_value());
  THES_CHECK(many.error().kind == ap::ParseErrorKind::excess_positional);
}

THES_TEST_CASE("A list split by a named argument is reported per list", "[argparse][parse]") {
  constexpr ap::ArgumentParser splitting{
    ap::ProgramInfo{.name = "split"},
    ap::flag<"verbose">("-v"),
    ap::list<"first">().exactly(2),
    ap::list<"second">().between(1, 2),
  };
  // The first list is closed by its bound before the flag, so only the second one is split.
  const auto result = splitting.parse(std::array{"1", "2", "3", "-v", "4"});
  THES_REQUIRE(!result.has_value());
  THES_CHECK(result.error().kind == ap::ParseErrorKind::split_values);
  THES_CHECK(result.error().argument == "SECOND");
}

THES_TEST_CASE("A bounded list is neither bracketed nor silent", "[argparse][parse]") {
  const std::string help = capture_help(cropping);
  THES_CHECK(help.find("Usage: crop [-h] [-v] CORNER... OUTPUT") != std::string::npos);
  THES_CHECK(help.find("The x and y of the corner (count: 2)") != std::string::npos);
}

/** `text` with every ANSI escape sequence removed. */
std::string without_escapes(std::string_view text) {
  std::string out{};
  for (std::size_t i = 0; i < text.size(); ++i) {
    if (text[i] != '\x1b') {
      out.push_back(text[i]);
      continue;
    }
    // A CSI sequence runs until its final byte, which for the ones used here is “m”.
    while (i < text.size() && text[i] != 'm') {
      ++i;
    }
  }
  return out;
}

THES_TEST_CASE("A style is written and leaves the layout alone", "[argparse][help]") {
  const std::string plain = capture_help(parser);
  const std::string styled = capture_help(parser.styled(ap::ansi_style));

  // Styling is off unless it is asked for, so redirected output stays free of escape sequences.
  THES_CHECK(plain.find('\x1b') == std::string::npos);
  THES_CHECK(styled.find('\x1b') != std::string::npos);
  // The column widths are measured over the text alone, so the two differ in nothing else.
  THES_CHECK(plain == without_escapes(styled));
}

THES_TEST_CASE("A style carries over to the parsers built from one", "[argparse][help]") {
  constexpr auto styled = ap::ArgumentParser{ap::ProgramInfo{.name = "demo"}}
                            .styled(ap::ansi_style)
                            .add(ap::flag<"quiet">("-q"))
                            .merge(ap::ArgumentGroup{ap::flag<"force">("-f")});
  THES_CHECK(capture_help(styled).find('\x1b') != std::string::npos);
}

THES_TEST_CASE("The built-in messages end in a full stop", "[argparse][help]") {
  const std::string help = capture_help(parser);
  THES_CHECK(help.find("Show this help text and exit.\n") != std::string::npos);
  THES_CHECK(help.find("Show the version and exit.\n") != std::string::npos);

  const auto result = parse(std::array{"in.txt", "--nope"});
  THES_REQUIRE(!result.has_value());
  const std::string message =
    capture([&](std::FILE* file) { parser.print_error(result.error(), file, "demo"); });
  // The message is one sentence, and the full stop stands outside the quotation marks, which
  // enclose the offending token and nothing more.
  THES_CHECK(message.starts_with("Error: Unknown argument “--nope”.\n"));
}

THES_TEST_CASE("A required named argument is marked as such", "[argparse][help]") {
  constexpr ap::ArgumentParser marking{
    ap::ProgramInfo{.name = "convert"},
    ap::positional<"input">().help("The file to read"),
    ap::option<"format">("-f").required().help("The output format"),
    ap::option<"mode">("-m").choices("fast"sv, "slow"sv).required().help("The mode"),
    ap::option<"level", int>("-l").default_to(3).help("The effort"),
  };
  const std::string help = capture_help(marking);
  THES_CHECK(help.find("The output format (required)") != std::string::npos);
  THES_CHECK(help.find("The mode (choices: fast, slow) (required)") != std::string::npos);
  // A default and being required are mutually exclusive, so only one of the two is ever shown.
  THES_CHECK(help.find("The effort (default: 3)") != std::string::npos);
  // A positional argument is required unless it says otherwise, so it goes unmarked.
  THES_CHECK(help.find("The file to read\n") != std::string::npos);
}

THES_TEST_CASE("A required list demands at least one value", "[argparse][parse]") {
  const auto empty = collecting.parse(std::array<const char*, 0>{});
  THES_REQUIRE(!empty.has_value());
  THES_CHECK(empty.error().kind == ap::ParseErrorKind::missing_argument);
  THES_CHECK(empty.error().argument == "INPUT-FILES");

  constexpr std::array tokens{"a"};
  const auto given = collecting.parse(tokens);
  THES_REQUIRE(given.has_value());
  // Being required rather than defaulted, the value needs no `std::optional` around it.
  static_assert(std::same_as<decltype(given->get<"input-files">()), const ap::TokenSpan&>);
  THES_CHECK(given->get<"input-files">().size() == 1);
}

THES_TEST_CASE("A double dash ends the named arguments", "[argparse][parse]") {
  constexpr std::array tokens{"--", "-c", "in.txt"};
  const auto result = parse(tokens);
  THES_REQUIRE(result.has_value());
  THES_CHECK(result->get<"input">() == "-c");
  THES_CHECK(result->get<"output">() == "in.txt");
  THES_CHECK(result->get<"count">() == 1);
}

THES_TEST_CASE("A negative number is a value rather than a name", "[argparse][parse]") {
  constexpr std::array tokens{"in.txt", "--count", "-3"};
  const auto result = parse(tokens);
  THES_REQUIRE(result.has_value());
  THES_CHECK(result->get<"count">() == -3);
}

//==================================================================================================
// Errors
//==================================================================================================

/** The kind of error `tokens` produces, or `std::nullopt` if they parse. */
constexpr std::optional<ap::ParseErrorKind> error_kind(std::span<const char* const> tokens) {
  const auto result = parse(tokens);
  if (result.has_value()) {
    return std::nullopt;
  }
  return result.error().kind;
}

THES_TEST_CASE("Malformed command lines are reported", "[argparse][error]") {
  THES_CHECK(error_kind(std::array{"in.txt", "--nope"}) == ap::ParseErrorKind::unknown_argument);
  THES_CHECK(error_kind(std::array{"in.txt", "-z"}) == ap::ParseErrorKind::unknown_argument);
  THES_CHECK(error_kind(std::array{"in.txt", "--count"}) == ap::ParseErrorKind::missing_value);
  THES_CHECK(error_kind(std::array{"in.txt", "--count=x"}) == ap::ParseErrorKind::invalid_value);
  THES_CHECK(error_kind(std::array{"in.txt", "--dry-run=1"}) ==
             ap::ParseErrorKind::unexpected_value);
  THES_CHECK(error_kind(std::array{"in.txt", "--mode=quick"}) ==
             ap::ParseErrorKind::invalid_choice);
  THES_CHECK(error_kind(std::array<const char*, 0>{}) == ap::ParseErrorKind::missing_argument);
}

THES_TEST_CASE("An error names the argument it concerns", "[argparse][error]") {
  constexpr std::array tokens{"in.txt", "-c", "x"};
  const auto result = parse(tokens);
  THES_REQUIRE(!result.has_value());
  THES_CHECK(result.error().argument == "--count");
  THES_CHECK(result.error().value == "x");
}

THES_TEST_CASE("Help and version are requested rather than parsed", "[argparse][error]") {
  THES_CHECK(error_kind(std::array{"--help"}) == ap::ParseErrorKind::help_requested);
  THES_CHECK(error_kind(std::array{"-h"}) == ap::ParseErrorKind::help_requested);
  THES_CHECK(error_kind(std::array{"--version"}) == ap::ParseErrorKind::version_requested);

  const auto result = parse(std::array{"--help"});
  THES_REQUIRE(!result.has_value());
  THES_CHECK(result.error().is_request());
}

//==================================================================================================
// Declaration
//==================================================================================================

THES_TEST_CASE("A required positional yields an unwrapped value", "[argparse][declaration]") {
  using Values = decltype(parser)::Values;
  static_assert(std::same_as<decltype(std::declval<Values&>().get<"input">()), std::string_view&>);
  static_assert(
    std::same_as<decltype(std::declval<Values&>().get<"scale">()), std::optional<double>&>);
  static_assert(std::same_as<decltype(std::declval<Values&>().get<"verbose">()), unsigned&>);
  static_assert(Values::contains<"input">);
  static_assert(!Values::contains<"nothing">);
  THES_CHECK(decltype(parser)::argument_num == 8);
}

THES_TEST_CASE("A parser can be assembled one argument at a time", "[argparse][declaration]") {
  constexpr auto built = ap::ArgumentParser{ap::ProgramInfo{.name = "built"}}
                           .add(ap::positional<"first", int>())
                           .add(ap::flag<"second">("-s"));
  constexpr std::array tokens{"5", "-s"};
  const auto result = built.parse(tokens);
  THES_REQUIRE(result.has_value());
  THES_CHECK(result->get<"first">() == 5);
  THES_CHECK(result->get<"second">());
}

//==================================================================================================
// Merging
//==================================================================================================

/** The arguments one concern of a program contributes. */
constexpr auto logging_arguments() {
  return ap::ArgumentGroup{
    ap::counter<"verbose">("-v").help("Increase verbosity"),
    ap::flag<"quiet">("-q").help("Say nothing at all"),
  }
    .titled("Logging");
}

/** The arguments another concern contributes, itself put together from two groups. */
constexpr auto io_arguments() {
  return ap::ArgumentGroup{ap::positional<"input">().help("The file to read")}
    .merge(ap::ArgumentGroup{ap::option<"output">("-o").default_to("out.txt"sv)}.titled("Output"))
    .add(ap::flag<"force">("-f").help("Overwrite the output"));
}

inline constexpr auto merged_parser =
  ap::ArgumentParser{ap::ProgramInfo{.name = "merged", .description = "Merged contributions."}}
    .merge(io_arguments(), logging_arguments())
    .add(ap::flag<"dry-run">("-n"));

THES_TEST_CASE("Merged groups parse as one declaration", "[argparse][merge]") {
  constexpr std::array tokens{"-vv", "in.txt", "-o", "written.txt", "-nf"};
  const auto result = merged_parser.parse(tokens);
  THES_REQUIRE(result.has_value());
  THES_CHECK(result->get<"verbose">() == 2U);
  THES_CHECK(result->get<"input">() == "in.txt");
  THES_CHECK(result->get<"output">() == "written.txt");
  THES_CHECK(result->get<"force">());
  THES_CHECK(result->get<"dry-run">());
  THES_CHECK(!result->get<"quiet">());
}

THES_TEST_CASE("Merging preserves the declaration order", "[argparse][merge]") {
  static_assert(decltype(merged_parser)::argument_num == 6);
  constexpr std::array tokens{"first"};
  const auto result = merged_parser.parse(tokens);
  THES_REQUIRE(result.has_value());
  THES_CHECK(result->get<"input">() == "first");
  THES_CHECK(result->get<"output">() == "out.txt");
}

THES_TEST_CASE("A group heads its own section in the help text", "[argparse][merge]") {
  const std::string help = capture_help(merged_parser);
  THES_CHECK(help.find("\nOptions:\n") != std::string::npos);
  THES_CHECK(help.find("\nLogging:\n") != std::string::npos);
  THES_CHECK(help.find("\nOutput:\n") != std::string::npos);
  // The innermost group a contribution passed through decides its section.
  THES_CHECK(help.find("Output:\n  -o, --output OUTPUT") != std::string::npos);
  THES_CHECK(help.find("Logging:\n  -v, --verbose") != std::string::npos);
}

//==================================================================================================
// Recursive constructor
//==================================================================================================

inline constexpr auto empty_recursive_parser = ap::ArgumentParser{
  ap::ProgramInfo{.name = "merged", .description = "Merged contributions."},
  ap::ArgumentGroup{},
};
static_assert(decltype(empty_recursive_parser)::argument_num == 0);

inline constexpr auto recursive_parser = ap::ArgumentParser{
  ap::ProgramInfo{.name = "merged", .description = "Merged contributions."},
  ap::ArgumentGroup{ap::counter<"verbose">()},
};
static_assert(decltype(recursive_parser)::argument_num == 1);

/** A group put together from groups, which contributes the arguments in them rather than itself. */
constexpr auto nested_arguments() {
  return ap::ArgumentGroup{
    ap::ArgumentGroup{},
    ap::ArgumentGroup{ap::positional<"input">().help("The file to read")},
    ap::flag<"force">("-f"),
    ap::ArgumentGroup{ap::ArgumentGroup{ap::counter<"verbose">("-v")}}.titled("Logging"),
  };
}

THES_TEST_CASE("A group is declared from groups as well as arguments", "[argparse][merge]") {
  constexpr auto nested = nested_arguments();
  static_assert(decltype(nested)::argument_num == 3);

  constexpr auto nested_parser = ap::ArgumentParser{ap::ProgramInfo{.name = "nested"}, nested};
  static_assert(decltype(nested_parser)::argument_num == 3);
  constexpr std::array tokens{"-vf", "in.txt"};
  const auto result = nested_parser.parse(tokens);
  THES_REQUIRE(result.has_value());
  THES_CHECK(result->get<"input">() == "in.txt");
  THES_CHECK(result->get<"force">());
  THES_CHECK(result->get<"verbose">() == 1U);
  // A group nested in another still heads the section it was given.
  THES_CHECK(capture_help(nested_parser).find("Logging:\n  -v, --verbose") != std::string::npos);
}

THES_TEST_CASE("Nesting and merging arrive at the same declaration", "[argparse][merge]") {
  constexpr auto nested = nested_arguments();
  constexpr auto assembled = ap::ArgumentGroup{}
                               .merge(ap::ArgumentGroup{ap::positional<"input">()})
                               .add(ap::flag<"force">("-f"))
                               .merge(ap::ArgumentGroup{ap::counter<"verbose">("-v")});
  static_assert(std::same_as<decltype(nested)::Arguments, decltype(assembled)::Arguments>);
}

//==================================================================================================
// Declaration
//==================================================================================================

THES_TEST_CASE("Long names default to the argument name", "[argparse][declaration]") {
  constexpr auto plain = ap::ArgumentParser{ap::ProgramInfo{}, ap::flag<"quiet">()};
  constexpr std::array tokens{"--quiet"};
  const auto result = plain.parse(tokens);
  THES_REQUIRE(result.has_value());
  THES_CHECK(result->get<"quiet">());
}
} // namespace

THES_TEST_MAIN()
