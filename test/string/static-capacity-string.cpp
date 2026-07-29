// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <string_view>

#include "thesauros/string/character-tools.hpp"
#include "thesauros/string/static-capacity-string.hpp"
#include "thesauros/test/test.hpp"

namespace {
using Str = thes::StaticCapacityString<16>;

//==================================================================================================
// Compile-time evaluation
//==================================================================================================

static_assert(Str::capacity == 16);
static_assert(Str{}.empty());
static_assert(Str{"abc"}.size() == 3);
static_assert(Str{"abc"}.view() == "abc");
static_assert(std::string_view{Str{"abc"}} == "abc");

// The deduction guide sizes the string to fit the literal exactly, minus the null terminator.
static_assert(decltype(thes::StaticCapacityString{"abcd"})::capacity == 4);

//==================================================================================================
// Construction
//==================================================================================================

/** Checks the three constructors and the implicit conversion to `std::string_view`. */
THES_TEST_CASE("construction", "[string][static-capacity-string]") {
  const Str empty{};
  THES_CHECK(empty.empty());
  THES_CHECK(empty.size() == 0);
  THES_CHECK(empty.length() == 0);
  THES_CHECK(empty.view().empty());
  THES_CHECK(std::strlen(empty.c_str()) == 0);

  const Str literal{"thesauros"};
  THES_CHECK(literal.size() == 9);
  THES_CHECK(literal.view() == "thesauros");
  THES_CHECK(std::strlen(literal.c_str()) == 9);

  const Str repeated(4, 'x');
  THES_CHECK(repeated.size() == 4);
  THES_CHECK(repeated.view() == "xxxx");

  const Str from_view{std::string_view{"view"}};
  THES_CHECK(from_view.size() == 4);
  THES_CHECK(from_view.view() == "view");

  // A string exactly filling the capacity is still null-terminated.
  const Str full(Str::capacity, 'y');
  THES_CHECK(full.size() == Str::capacity);
  THES_CHECK(std::strlen(full.c_str()) == Str::capacity);
}

//==================================================================================================
// Element access
//==================================================================================================

/** Checks indexed access, `at`, `front` and `back`, both const and mutable. */
THES_TEST_CASE("element access", "[string][static-capacity-string]") {
  Str text{"abcd"};
  const Str& ctext = text;

  THES_CHECK(text[0] == 'a');
  THES_CHECK(ctext[3] == 'd');
  THES_CHECK(text.front() == 'a');
  THES_CHECK(ctext.front() == 'a');
  THES_CHECK(text.back() == 'd');
  THES_CHECK(ctext.back() == 'd');

  text[1] = 'B';
  THES_CHECK(ctext.view() == "aBcd");
  text.front() = 'A';
  text.back() = 'D';
  THES_CHECK(ctext.view() == "ABcD");

  THES_CHECK(text.at(2) == 'c');
  THES_CHECK(ctext.at(2) == 'c');
  THES_CHECK_THROWS_AS(text.at(4), std::out_of_range);
  THES_CHECK_THROWS_AS(ctext.at(100), std::out_of_range);
}

/** Checks that iteration covers exactly the current contents. */
THES_TEST_CASE("iteration", "[string][static-capacity-string]") {
  Str text{"abc"};
  const Str& ctext = text;

  THES_CHECK(text.end() - text.begin() == 3);
  THES_CHECK(ctext.cend() - ctext.cbegin() == 3);
  THES_CHECK(std::equal(ctext.begin(), ctext.end(), "abc"));

  std::ranges::transform(text, text.begin(), [](char c) { return static_cast<char>(c - 32); });
  THES_CHECK(ctext.view() == "ABC");

  const Str empty{};
  THES_CHECK(empty.begin() == empty.end());
}

//==================================================================================================
// Modifiers
//==================================================================================================

/** Checks that every modifier keeps the buffer null-terminated. */
THES_TEST_CASE("modifiers keep the string terminated", "[string][static-capacity-string]") {
  Str text{"abcdef"};

  text.pop_back();
  THES_CHECK(text.view() == "abcde");
  THES_CHECK(std::strlen(text.c_str()) == 5);

  text.push_back('X');
  THES_CHECK(text.view() == "abcdeX");
  THES_CHECK(std::strlen(text.c_str()) == 6);

  text.resize(3);
  THES_CHECK(text.view() == "abc");
  THES_CHECK(std::strlen(text.c_str()) == 3);

  text.resize(6, '.');
  THES_CHECK(text.view() == "abc...");
  THES_CHECK(std::strlen(text.c_str()) == 6);

  text.clear();
  THES_CHECK(text.empty());
  THES_CHECK(std::strlen(text.c_str()) == 0);
}

/** Checks `append` and the two `operator+=` overloads. */
THES_TEST_CASE("appending", "[string][static-capacity-string]") {
  Str text{};

  text.append("abc");
  THES_CHECK(text.view() == "abc");

  text += 'd';
  THES_CHECK(text.view() == "abcd");

  text += std::string_view{"ef"};
  THES_CHECK(text.view() == "abcdef");

  // Appending nothing is a no-op.
  text.append("");
  THES_CHECK(text.view() == "abcdef");
  THES_CHECK(std::strlen(text.c_str()) == 6);

  // `append` returns a reference, so calls chain.
  text.append("g").append("h");
  THES_CHECK(text.view() == "abcdefgh");
}

/** Checks `set_size`, which adopts characters written directly into `data()`. */
THES_TEST_CASE("set_size adopts directly written characters", "[string][static-capacity-string]") {
  Str text{};
  const char source[]{"1234"};
  std::copy(source, source + 4, text.data());
  text.set_size(4);

  THES_CHECK(text.size() == 4);
  THES_CHECK(text.view() == "1234");
  THES_CHECK(std::strlen(text.c_str()) == 4);

  // Shrinking via `set_size` moves the terminator without touching the rest of the buffer.
  text.set_size(2);
  THES_CHECK(text.view() == "12");
  THES_CHECK(std::strlen(text.c_str()) == 2);
}

//==================================================================================================
// Comparison and formatting
//==================================================================================================

/** Checks comparison against `std::string_view`. */
THES_TEST_CASE("comparison", "[string][static-capacity-string]") {
  const Str text{"abc"};

  THES_CHECK((text == std::string_view{"abc"}));
  THES_CHECK((text != std::string_view{"abd"}));
  THES_CHECK((text < std::string_view{"abd"}));
  THES_CHECK((text > std::string_view{"ab"}));
  THES_CHECK((text <= std::string_view{"abc"}));
  THES_CHECK((text >= std::string_view{"abc"}));
}

/** Checks that `format_as` exposes the current contents, not the whole buffer. */
THES_TEST_CASE("format_as yields the current contents", "[string][static-capacity-string]") {
  Str text{"abcdef"};
  text.resize(3);
  THES_CHECK(format_as(text) == std::string_view{"abc"});
}

//==================================================================================================
// Character tools
//==================================================================================================

static_assert(thes::is_uppercase('A'));
static_assert(thes::is_uppercase('Z'));
static_assert(!thes::is_uppercase('a'));
static_assert(!thes::is_uppercase('@'));
static_assert(thes::to_lowercase('A') == 'a');
static_assert(thes::to_lowercase('a') == 'a');

/** Checks `is_uppercase` and `to_lowercase` over the whole ASCII range. */
THES_TEST_CASE("character classification covers ASCII", "[string][character-tools]") {
  for (int i = 0; i < 128; ++i) {
    const auto c = static_cast<char>(i);
    const bool upper = (c >= 'A' && c <= 'Z');
    THES_CHECK(thes::is_uppercase(c) == upper);

    const char expected = upper ? static_cast<char>(c - 'A' + 'a') : c;
    THES_CHECK(thes::to_lowercase(c) == expected);
  }

  // Characters that merely look adjacent to the letter range are left alone.
  THES_CHECK(thes::to_lowercase('@') == '@');
  THES_CHECK(thes::to_lowercase('[') == '[');
  THES_CHECK(thes::to_lowercase('0') == '0');
}
} // namespace

THES_TEST_MAIN()
