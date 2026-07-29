// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "thesauros/test/test.hpp"
#include "thesauros/types/primitives.hpp"
#include "thesauros/utility/info-result.hpp"
#include "thesauros/utility/optional.hpp"
#include "thesauros/utility/value-optional.hpp"

namespace {
//==================================================================================================
// thes::Optional
//==================================================================================================

/** Checks that `Optional` interoperates with `std::optional` in both directions. */
THES_TEST_CASE("Optional converts from std::optional", "[utility][optional]") {
  const std::optional<int> present{7};
  const std::optional<int> absent{};

  const thes::Optional<int> from_present{present};
  const thes::Optional<int> from_absent{absent};
  THES_CHECK(from_present.has_value());
  THES_CHECK(from_present.value() == 7);
  THES_CHECK(!from_absent.has_value());

  // The inherited `std::optional` interface remains available.
  THES_CHECK(from_present.value_or(0) == 7);
  THES_CHECK(from_absent.value_or(3) == 3);
  THES_CHECK(*from_present == 7);
}

/** Checks that `and_then` chains only over engaged optionals. */
THES_TEST_CASE("Optional::and_then chains", "[utility][optional]") {
  const auto halve = [](int v) {
    return (v % 2 == 0) ? thes::Optional<int>{v / 2} : thes::Optional<int>{};
  };

  THES_CHECK(thes::Optional<int>{8}.and_then(halve).value() == 4);
  THES_CHECK(thes::Optional<int>{8}.and_then(halve).and_then(halve).value() == 2);
  THES_CHECK(!thes::Optional<int>{6}.and_then(halve).and_then(halve).has_value());
  THES_CHECK(!thes::Optional<int>{}.and_then(halve).has_value());

  // The lvalue overload passes a mutable reference through.
  thes::Optional<int> value{4};
  const auto doubled = value.and_then([](int& v) { return thes::Optional<int>{2 * v}; });
  THES_CHECK(doubled.value() == 8);
}

/** Checks that `transform` maps the contained value and may change the value type. */
THES_TEST_CASE("Optional::transform maps the value", "[utility][optional]") {
  const thes::Optional<int> present{21};
  const thes::Optional<int> absent{};

  THES_CHECK(present.transform([](int v) { return 2 * v; }).value() == 42);
  THES_CHECK(!absent.transform([](int v) { return 2 * v; }).has_value());

  // The result type follows the callable’s return type, not the source’s value type.
  const auto as_string = present.transform([](int v) { return std::to_string(v); });
  THES_CHECK(as_string.value() == "21");
  THES_CHECK(!absent.transform([](int v) { return std::to_string(v); }).has_value());
}

/** Checks that `or_else` supplies a fallback exactly when the optional is empty. */
THES_TEST_CASE("Optional::or_else supplies a fallback", "[utility][optional]") {
  const auto fallback = [] { return thes::Optional<int>{99}; };

  THES_CHECK(thes::Optional<int>{1}.or_else(fallback).value() == 1);
  THES_CHECK(thes::Optional<int>{}.or_else(fallback).value() == 99);

  // The rvalue overload moves the engaged value through unchanged.
  thes::Optional<std::string> text{"kept"};
  const auto kept = std::move(text).or_else([] { return thes::Optional<std::string>{"replaced"}; });
  THES_CHECK(kept.value() == "kept");
}

/** Checks that `value_or_else` calls its callable lazily, only for an empty optional. */
THES_TEST_CASE("Optional::value_or_else is lazy", "[utility][optional]") {
  int calls = 0;
  const auto fallback = [&calls] {
    ++calls;
    return 5;
  };

  const thes::Optional<int> present{1};
  THES_CHECK(present.value_or_else(fallback) == 1);
  THES_CHECK(calls == 0);

  const thes::Optional<int> absent{};
  THES_CHECK(absent.value_or_else(fallback) == 5);
  THES_CHECK(calls == 1);

  THES_CHECK(thes::Optional<int>{2}.value_or_else(fallback) == 2);
  THES_CHECK(calls == 1);
  THES_CHECK(thes::Optional<int>{}.value_or_else(fallback) == 5);
  THES_CHECK(calls == 2);
}

//==================================================================================================
// thes::ValueOptional
//==================================================================================================

using Sentinel = thes::ValueOptional<int, -1>;
using MaxOpt = thes::MaxOptional<thes::u32>;

static_assert(Sentinel::empty_value == -1);
static_assert(!Sentinel{}.has_value());
static_assert(Sentinel{3}.has_value());
static_assert(*Sentinel{3} == 3);

/** Checks that the sentinel value is what distinguishes the empty state. */
THES_TEST_CASE("ValueOptional uses its sentinel as the empty state", "[utility][value-optional]") {
  const Sentinel empty{};
  THES_CHECK(empty.is_empty());
  THES_CHECK(!empty.has_value());
  THES_CHECK(*empty == Sentinel::empty_value);

  const Sentinel value{5};
  THES_CHECK(!value.is_empty());
  THES_CHECK(value.has_value());
  THES_CHECK(value.value() == 5);
  THES_CHECK(*value == 5);

  // Zero is an ordinary value here, since the sentinel is `-1`.
  const Sentinel zero{0};
  THES_CHECK(zero.has_value());
  THES_CHECK(zero.value() == 0);
}

/** Checks `set` and `clear`. */
THES_TEST_CASE("ValueOptional can be set and cleared", "[utility][value-optional]") {
  Sentinel value{};
  THES_CHECK(value.is_empty());

  value.set(4);
  THES_CHECK(value.has_value());
  THES_CHECK(value.value() == 4);

  value.set(9);
  THES_CHECK(value.value() == 9);

  value.clear();
  THES_CHECK(value.is_empty());
  THES_CHECK(!value.has_value());
}

/** Checks that `value_run` invokes its callable only when a value is present. */
THES_TEST_CASE("ValueOptional::value_run runs conditionally", "[utility][value-optional]") {
  int seen = 0;
  int calls = 0;
  const auto record = [&seen, &calls](int v) {
    seen = v;
    ++calls;
  };

  Sentinel empty{};
  empty.value_run(record);
  THES_CHECK(calls == 0);

  Sentinel value{6};
  value.value_run(record);
  THES_CHECK(calls == 1);
  THES_CHECK(seen == 6);
}

/** Checks the monadic operations, which all short-circuit on the empty state. */
THES_TEST_CASE("ValueOptional monadic operations", "[utility][value-optional]") {
  const auto negate_if_even = [](int v) { return (v % 2 == 0) ? Sentinel{-v} : Sentinel{}; };

  THES_CHECK(Sentinel{4}.and_then(negate_if_even).value() == -4);
  THES_CHECK(Sentinel{3}.and_then(negate_if_even).is_empty());
  THES_CHECK(Sentinel{}.and_then(negate_if_even).is_empty());

  THES_CHECK(Sentinel{4}.transform([](int v) { return 2 * v; }).value() == 8);
  THES_CHECK(Sentinel{}.transform([](int v) { return 2 * v; }).is_empty());

  THES_CHECK(Sentinel{4}.or_else([] { return Sentinel{7}; }).value() == 4);
  THES_CHECK(Sentinel{}.or_else([] { return Sentinel{7}; }).value() == 7);
  THES_CHECK(Sentinel{}.or_else([] { return Sentinel{}; }).is_empty());
}

/** Checks `MaxOptional`, whose sentinel is the maximum value of the underlying integer type. */
THES_TEST_CASE("MaxOptional uses the type maximum as its sentinel", "[utility][value-optional]") {
  static_assert(MaxOpt::empty_value == thes::u32{0xFFFFFFFF});

  const MaxOpt empty{};
  THES_CHECK(empty.is_empty());

  const MaxOpt zero{0};
  THES_CHECK(zero.has_value());
  THES_CHECK(zero.value() == 0);

  const MaxOpt large{MaxOpt::empty_value - 1};
  THES_CHECK(large.has_value());
  THES_CHECK(large.value() == MaxOpt::empty_value - 1);
}

//==================================================================================================
// thes::InfoResult
//==================================================================================================

enum struct Status : thes::u8 { ok, warning, error };
using Checked = thes::InfoResult<int, Status, Status::ok>;

static_assert(Checked{3, Status::ok}.is_valid());
static_assert(!Checked{3, Status::warning}.is_valid());
static_assert(*Checked{3, Status::ok} == 3);

/** Checks that validity is decided solely by comparing the info against the expected value. */
THES_TEST_CASE("InfoResult reports validity from its info", "[utility][info-result]") {
  const Checked good{42, Status::ok};
  THES_CHECK(good.is_valid());
  THES_CHECK(*good == 42);
  THES_CHECK(good.raw() == 42);
  THES_CHECK(good.value_or(0) == 42);
  THES_CHECK(good.valid_value() == 42);

  const Checked bad{42, Status::error};
  THES_CHECK(!bad.is_valid());
  // The value is still readable through `raw`, which performs no check.
  THES_CHECK(bad.raw() == 42);
  THES_CHECK(bad.value_or(0) == 0);
  THES_CHECK_THROWS_AS(bad.valid_value(), std::runtime_error);
}
} // namespace

THES_TEST_MAIN()
