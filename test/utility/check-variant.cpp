// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <concepts>
#include <stdexcept>
#include <string>
#include <variant>

#include "thesauros/test/test.hpp"
#include "thesauros/utility/check-variant.hpp"

namespace {
/** An error alternative, which the checker turns into an exception. */
struct NotFound {
  std::string what{};
};
/** A second error alternative, to check that several can be filtered out at once. */
struct Denied {
  int code{};
};

/**
 * Maps each alternative to either an exception, which makes `check_variant` throw it and drop the
 * alternative from the result, or to anything else, which lets the value through.
 */
inline constexpr auto checker = []<typename T>(const T& value) {
  if constexpr (std::same_as<T, NotFound>) {
    return std::runtime_error{value.what};
  } else if constexpr (std::same_as<T, Denied>) {
    return std::logic_error{"denied"};
  } else {
    return 0;
  }
};

//==================================================================================================
// Passing values through
//==================================================================================================

/** Checks that the sole surviving alternative collapses to its own type. */
THES_TEST_CASE("a single surviving alternative is unwrapped", "[utility][check-variant]") {
  using Var = std::variant<int, NotFound>;

  const auto value = thes::check_variant(Var{7}, checker);
  static_assert(std::same_as<decltype(value), const int>);
  THES_CHECK(value == 7);
}

/** Checks that several surviving alternatives stay in a variant of just those. */
THES_TEST_CASE("surviving alternatives stay in a variant", "[utility][check-variant]") {
  using Var = std::variant<int, double, NotFound>;

  const auto from_int = thes::check_variant(Var{7}, checker);
  static_assert(std::same_as<decltype(from_int), const std::variant<int, double>>);
  THES_REQUIRE(from_int.index() == 0);
  THES_CHECK(std::get<int>(from_int) == 7);

  const auto from_double = thes::check_variant(Var{2.5}, checker);
  THES_REQUIRE(from_double.index() == 1);
  THES_CHECK(std::get<double>(from_double) == 2.5);
}

//==================================================================================================
// Throwing on error alternatives
//==================================================================================================

/** Checks that an error alternative throws the exception the checker built for it. */
THES_TEST_CASE("an error alternative throws", "[utility][check-variant]") {
  using Var = std::variant<int, NotFound>;

  THES_CHECK_THROWS_AS(thes::check_variant(Var{NotFound{"missing"}}, checker), std::runtime_error);

  // The exception carries the message the checker put into it.
  bool seen = false;
  try {
    (void)thes::check_variant(Var{NotFound{"missing"}}, checker);
  } catch (const std::runtime_error& error) {
    seen = std::string{error.what()} == "missing";
  }
  THES_CHECK(seen);
}

/** Checks that each error alternative throws its own exception type. */
THES_TEST_CASE("each error alternative throws its own type", "[utility][check-variant]") {
  using Var = std::variant<int, NotFound, Denied>;

  THES_CHECK_THROWS_AS(thes::check_variant(Var{NotFound{"gone"}}, checker), std::runtime_error);
  THES_CHECK_THROWS_AS(thes::check_variant(Var{Denied{403}}, checker), std::logic_error);

  // The non-error alternative is unaffected, and is the only one left in the result.
  const auto value = thes::check_variant(Var{1}, checker);
  static_assert(std::same_as<decltype(value), const int>);
  THES_CHECK(value == 1);
}

/** Checks that a variant whose alternatives are all fine is passed through untouched. */
THES_TEST_CASE("a variant without errors passes through", "[utility][check-variant]") {
  using Var = std::variant<int, double>;

  const auto from_int = thes::check_variant(Var{3}, checker);
  THES_CHECK(std::get<int>(from_int) == 3);

  const auto from_double = thes::check_variant(Var{4.5}, checker);
  THES_CHECK(std::get<double>(from_double) == 4.5);
}
} // namespace

THES_TEST_MAIN()
