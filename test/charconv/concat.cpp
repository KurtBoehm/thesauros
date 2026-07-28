// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <cstddef>
#include <limits>
#include <string>
#include <string_view>

#include "thesauros/charconv/concat.hpp"
#include "thesauros/string/static-capacity-string.hpp"
#include "thesauros/test/test.hpp"

namespace {
using namespace std::string_view_literals;

THES_TEST_CASE("cat with no arguments yields an empty string", "[charconv][concat]") {
  THES_CHECK(thes::cat().empty());
}

THES_TEST_CASE("cat copies string-like arguments verbatim", "[charconv][concat]") {
  THES_CHECK(thes::cat("abc") == "abc");
  THES_CHECK(thes::cat("abc"sv, std::string{"def"}) == "abcdef");
  THES_CHECK(thes::cat(thes::StaticCapacityString<8>{"xy"sv}) == "xy");
}

THES_TEST_CASE("cat appends char as a single character, not a number", "[charconv][concat]") {
  THES_CHECK(thes::cat('a') == "a");
  THES_CHECK(thes::cat("x", '=', 'y') == "x=y");
}

THES_TEST_CASE("cat renders bool as true/false", "[charconv][concat]") {
  THES_CHECK(thes::cat(true) == "true");
  THES_CHECK(thes::cat(false) == "false");
}

THES_TEST_CASE("cat converts integers, including limits", "[charconv][concat]") {
  THES_CHECK(thes::cat(0) == "0");
  THES_CHECK(thes::cat(42) == "42");
  THES_CHECK(thes::cat(-7) == "-7");
  THES_CHECK(thes::cat(std::numeric_limits<int>::min()) ==
             std::to_string(std::numeric_limits<int>::min()));
  THES_CHECK(thes::cat(std::size_t{123}) == "123");
}

THES_TEST_CASE("cat converts floating-point values round-trippably", "[charconv][concat]") {
  THES_CHECK(thes::cat(3.5) == "3.5");
  THES_CHECK(thes::cat(-2.25F) == "-2.25");
}

THES_TEST_CASE("cat mixes types in one call", "[charconv][concat]") {
  THES_CHECK(thes::cat("fread failed: ", 3, " != ", std::size_t{8}) == "fread failed: 3 != 8");
  THES_CHECK(thes::cat("cpu", 11) == "cpu11");
}
} // namespace

THES_TEST_MAIN()
