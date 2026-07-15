// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_TEST_ASSERT_HPP
#define INCLUDE_THESAUROS_TEST_ASSERT_HPP

#include <cstdlib>
#include <source_location>

#include "thesauros/format.hpp"

namespace thes::test {
#define THES_ALWAYS_ASSERT(expr) ((expr) ? void(0) : ::thes::test::assert_fail(#expr, [] {}))
#define THES_ALWAYS_ASSERT_ACTION(expr, action) \
  ((expr) ? void(0) : ::thes::test::assert_fail(#expr, action))

#ifdef NDEBUG
#define THES_ASSERT(expr)
#define THES_ASSERT_ACTION(expr, action)
#else
#define THES_ASSERT(expr) THES_ALWAYS_ASSERT(expr)
#define THES_ASSERT_ACTION(expr, action) THES_ALWAYS_ASSERT_ACTION(expr, action)
#endif

/** Fails the current test, prints location, executes `fun`, then aborts. */
inline void assert_fail(const char* assertion, auto fun,
                        const std::source_location location = std::source_location::current()) {
  fmt::print(stderr, "Assertion “{}” failed in {} @ {}:{}:{}\n", assertion,
             location.function_name(), location.file_name(), location.line(), location.column());
  fun();
  std::abort();
}
} // namespace thes::test

#endif // INCLUDE_THESAUROS_TEST_ASSERT_HPP
