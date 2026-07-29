// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <array>
#include <cstddef>
#include <string>
#include <string_view>

#include "thesauros/charconv/string-escape.hpp"
#include "thesauros/containers/bitset/dynamic.hpp"
#include "thesauros/containers/bitset/fixed.hpp"
#include "thesauros/containers/bitset/static.hpp"
#include "thesauros/format.hpp"
#include "thesauros/ranges/indices.hpp"
#include "thesauros/string/static-capacity-string.hpp"
#include "thesauros/string/static-string.hpp"
#include "thesauros/test/test.hpp"
#include "thesauros/types/primitives.hpp"
#include "thesauros/utility/multi-size.hpp"
#include "thesauros/utility/value-optional.hpp"

namespace {
/** Formats `value` with the default specification. */
template<typename T>
[[nodiscard]] std::string render(const T& value) {
  return fmt::format("{}", value);
}

//==================================================================================================
// Bitsets
//==================================================================================================

/**
 * Checks that a bitset renders most-significant bit first, so that reading it left to right matches
 * the usual binary notation rather than the storage order.
 */
THES_TEST_CASE("bitsets print from the top bit down", "[format][bitsets]") {
  thes::DynamicBitset<thes::u64> dynamic{5, false};
  dynamic.set(0);
  dynamic.set(2);
  THES_CHECK(render(dynamic) == "00101");

  thes::FixedBitset<8> fixed{5, false};
  fixed.set(0);
  fixed.set(2);
  THES_CHECK(render(fixed) == "00101");

  thes::StaticBitset<5> stat{};
  stat.set(0);
  stat.set(2);
  THES_CHECK(render(stat) == "00101");
}

/** Checks the all-set, none-set and empty extremes. */
THES_TEST_CASE("uniform bitsets print uniformly", "[format][bitsets]") {
  const thes::DynamicBitset<thes::u64> all_set{6, true};
  const thes::DynamicBitset<thes::u64> none_set{6, false};
  THES_CHECK(render(all_set) == "111111");
  THES_CHECK(render(none_set) == "000000");

  const thes::DynamicBitset<thes::u64> empty{};
  THES_CHECK(render(empty).empty());

  // A bitset spanning more than one chunk still prints as one continuous run of bits.
  const thes::FixedBitset<1> wide{20, true};
  THES_CHECK(render(wide) == std::string(20, '1'));
}

/** Checks that the padding specification applies to the rendered bit string. */
THES_TEST_CASE("bitsets honour a width specification", "[format][bitsets]") {
  thes::StaticBitset<3> bits{};
  bits.set(1);

  THES_CHECK(fmt::format("{:>6}", bits) == "   010");
  THES_CHECK(fmt::format("{:<6}", bits) == "010   ");
  THES_CHECK(fmt::format("{:^7}", bits) == "  010  ");
}

//==================================================================================================
// IotaRange
//==================================================================================================

/** Checks that an `IotaRange` prints as its half-open bounds rather than its elements. */
THES_TEST_CASE("an IotaRange prints as an interval", "[format][ranges]") {
  THES_CHECK(render(thes::views::indices(0, 4)) == "[0, 4)");
  THES_CHECK(render(thes::views::indices(3, 3)) == "[3, 3)");
  THES_CHECK(render(thes::views::indices(-2, 2)) == "[-2, 2)");

  // The specification pads the rendered interval as a whole.
  THES_CHECK(fmt::format("{:>10}", thes::views::indices(1, 12)) == "   [1, 12)");
  THES_CHECK(fmt::format("{:<10}", thes::views::indices(1, 12)) == "[1, 12)   ");
}

//==================================================================================================
// ValueOptional
//==================================================================================================

/** Checks that an engaged optional prints its value and an empty one the word `empty`. */
THES_TEST_CASE("a ValueOptional prints its value or `empty`", "[format][value-optional]") {
  using Opt = thes::ValueOptional<int, -1>;

  THES_CHECK(render(Opt{7}) == "7");
  THES_CHECK(render(Opt{0}) == "0");
  THES_CHECK(render(Opt{}) == "empty");

  using MaxOpt = thes::MaxOptional<thes::u16>;
  THES_CHECK(render(MaxOpt{42}) == "42");
  THES_CHECK(render(MaxOpt{}) == "empty");

  // The specification pads the rendering, whichever branch it took.
  THES_CHECK(fmt::format("{:>6}", Opt{7}) == "     7");
  THES_CHECK(fmt::format("{:<6}", Opt{}) == "empty ");
}

//==================================================================================================
// SubMultiSize
//==================================================================================================

/** Checks that a sub-box prints as its per-axis intervals, joined by a multiplication sign. */
THES_TEST_CASE("a SubMultiSize prints its axis ranges", "[format][multi-size]") {
  const thes::SubMultiSize<thes::u32, 2> sub{std::array<thes::u32, 2>{1, 2},
                                             std::array<thes::u32, 2>{2, 3}};
  THES_CHECK(render(sub) == "[1, 3)×[2, 5)");

  const thes::SubMultiSize<thes::u32, 3> cube{std::array<thes::u32, 3>{0, 0, 0},
                                              std::array<thes::u32, 3>{2, 2, 2}};
  THES_CHECK(render(cube) == "[0, 2)×[0, 2)×[0, 2)");
}

//==================================================================================================
// Escaped strings
//==================================================================================================

/** Checks that `escaped_string` renders control characters and quotes in escaped form. */
THES_TEST_CASE("escaped strings render their escapes", "[format][string-escape]") {
  THES_CHECK(render(thes::escaped_string(std::string_view{"plain"})) == "plain");
  THES_CHECK(render(thes::escaped_string(std::string_view{"a\nb"})) == "a\\nb");
  THES_CHECK(render(thes::escaped_string(std::string_view{"tab\there"})) == "tab\\there");
  THES_CHECK(render(thes::escaped_string(std::string_view{"say \"hi\""})) == "say \\\"hi\\\"");
  THES_CHECK(render(thes::escaped_string(std::string_view{"back\\slash"})) == "back\\\\slash");
  THES_CHECK(render(thes::escaped_string(std::string_view{""})).empty());
}

//==================================================================================================
// Strings
//==================================================================================================

/**
 * Checks that the string types print as text. Both are ranges of `char`, so without the
 * customizations in `format/` they would print as a bracketed list of characters instead.
 */
THES_TEST_CASE("the string types print as text", "[format][string]") {
  const thes::StaticCapacityString<8> capacity{"abc"};
  THES_CHECK(render(capacity) == "abc");
  THES_CHECK(fmt::format("{:>5}", capacity) == "  abc");

  // Only the current contents are printed, not the whole buffer.
  thes::StaticCapacityString<8> shortened{"abcdef"};
  shortened.resize(2);
  THES_CHECK(render(shortened) == "ab");

  const thes::StaticCapacityString<4> empty{};
  THES_CHECK(render(empty).empty());

  static constexpr thes::StaticString stat{"xyz"};
  THES_CHECK(render(stat) == "xyz");
}
} // namespace

THES_TEST_MAIN()
