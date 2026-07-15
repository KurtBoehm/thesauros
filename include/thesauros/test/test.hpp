// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_TEST_TEST_HPP
#define INCLUDE_THESAUROS_TEST_TEST_HPP

#include <concepts>
#include <cstddef>
#include <exception>
#include <source_location>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "thesauros/format.hpp"

namespace thes::test {
//==================================================================================================
// Core types
//==================================================================================================

/** The outcome of a single check or require. */
struct CheckResult {
  bool passed{};
  std::string expression{};
  std::string message{};
  std::source_location location{};
};

/** A single named test case registered via `THES_TEST_CASE`. */
struct TestCase {
  using Fn = void (*)();

  std::string_view name{};
  std::string_view tags{};
  Fn function{};
};

/** Thrown by `THES_REQUIRE` to abort the current test case after a failed check. */
struct AssertionFailure : std::exception {
  explicit AssertionFailure(std::string_view expression) : expression_{expression} {}

  [[nodiscard]] const char* what() const noexcept override {
    return expression_.c_str();
  }

private:
  std::string expression_{};
};

//==================================================================================================
// Registry
//==================================================================================================

/** Collects all registered test cases and runs them in registration order. */
struct Registry {
  static Registry& instance() {
    static Registry inst{};
    return inst;
  }

  void add(TestCase tc) {
    cases.push_back(tc);
  }

  void record(CheckResult res) {
    if (!res.passed) {
      ++current_failures;
    }
    results.push_back(std::move(res));
  }

  /** Runs every registered test case and returns a process exit code. */
  int run() {
    std::size_t total_checks{};
    std::size_t failed_checks{};
    std::size_t failed_cases{};

    for (const auto& tc : cases) {
      results.clear();
      current_failures = 0;

      fmt::print("[ RUN      ] {}\n", tc.name);
      try {
        tc.function();
      } catch (const std::exception& error) {
        fmt::print("  Uncaught exception: {}\n", error.what());
        ++current_failures;
      } catch (...) {
        fmt::print("  Uncaught exception of unknown type.\n");
        ++current_failures;
      }

      for (const auto& res : results) {
        ++total_checks;
        if (!res.passed) {
          ++failed_checks;
          fmt::print("  FAILED: {}\n", res.expression);
          if (!res.message.empty()) {
            fmt::print("    reason: {}\n", res.message);
          }
          fmt::print("    at {}:{}\n", res.location.file_name(), res.location.line());
        }
      }

      fmt::print("[{}] {}\n", current_failures > 0 ? "  FAILED  " : "       OK ", tc.name);
      failed_cases += current_failures > 0 ? 1 : 0;
    }

    fmt::print("\n");
    fmt::print("{}/{} test cases passed, {}/{} checks passed.\n", cases.size() - failed_cases,
               cases.size(), total_checks - failed_checks, total_checks);

    return failed_cases == 0 ? 0 : 1;
  }

  std::vector<TestCase> cases{};
  std::vector<CheckResult> results{};
  std::size_t current_failures{};
};

//==================================================================================================
// Registration helper
//==================================================================================================

/** Registers a `TestCase` with the `Registry` during static initialization. */
struct AutoRegister {
  AutoRegister(std::string_view name, std::string_view tags, TestCase::Fn function) noexcept {
    Registry::instance().add({.name = name, .tags = tags, .function = function});
  }
};

//--------------------------------------------------------------------------------------------------
// Comparison helpers
//--------------------------------------------------------------------------------------------------

/**
 * Compares `lhs` and `rhs` for equality, using `std::cmp_equal` for integral operands to avoid
 *  sign-compare warnings and pitfalls when their signedness differs.
 */
template<typename Lhs, typename Rhs>
constexpr bool safe_eq(const Lhs& lhs, const Rhs& rhs) {
  if constexpr (std::integral<Lhs> && std::integral<Rhs>) {
    return std::cmp_equal(lhs, rhs);
  } else {
    return lhs == rhs;
  }
}

/** Compares `lhs` and `rhs` for inequality, using `std::cmp_not_equal` for integral operands. */
template<typename Lhs, typename Rhs>
constexpr bool safe_ne(const Lhs& lhs, const Rhs& rhs) {
  if constexpr (std::integral<Lhs> && std::integral<Rhs>) {
    return std::cmp_not_equal(lhs, rhs);
  } else {
    return lhs != rhs;
  }
}

//==================================================================================================
// Expression decomposition
//==================================================================================================

/** The result of evaluating a (possibly decomposed) expression passed to `THES_CHECK`. */
struct DecomposedExpression {
  bool passed{};
  std::string message{};
};

/**
 * Captures the left-hand side of an expression passed to `THES_CHECK`/`THES_REQUIRE`, so that a
 * subsequent comparison operator, if any, can be intercepted and both operands recorded for a
 * descriptive failure message; converts directly to a `DecomposedExpression` if no comparison
 * operator follows, via `to_decomposed()`.
 */
template<typename Lhs>
struct ExpressionCapture {
  const Lhs& lhs;

  template<typename Rhs>
  requires(std::equality_comparable_with<Lhs, Rhs>)
  DecomposedExpression operator==(const Rhs& rhs) const {
    return compare(safe_eq(lhs, rhs), "==", rhs);
  }

  template<typename Rhs>
  requires(std::equality_comparable_with<Lhs, Rhs>)
  DecomposedExpression operator!=(const Rhs& rhs) const {
    return compare(safe_ne(lhs, rhs), "!=", rhs);
  }

  template<typename Rhs>
  requires(std::totally_ordered_with<Lhs, Rhs>)
  DecomposedExpression operator<(const Rhs& rhs) const {
    return compare(lhs < rhs, "<", rhs);
  }

  template<typename Rhs>
  requires(std::totally_ordered_with<Lhs, Rhs>)
  DecomposedExpression operator<=(const Rhs& rhs) const {
    return compare(lhs <= rhs, "<=", rhs);
  }

  template<typename Rhs>
  requires(std::totally_ordered_with<Lhs, Rhs>)
  DecomposedExpression operator>(const Rhs& rhs) const {
    return compare(lhs > rhs, ">", rhs);
  }

  template<typename Rhs>
  requires(std::totally_ordered_with<Lhs, Rhs>)
  DecomposedExpression operator>=(const Rhs& rhs) const {
    return compare(lhs >= rhs, ">=", rhs);
  }

private:
  template<typename Rhs>
  DecomposedExpression compare(bool passed, std::string_view op, const Rhs& rhs) const {
    std::string message{};
    if (!passed) {
      if constexpr (fmt::formattable<Lhs> && fmt::formattable<Rhs>) {
        message = fmt::format("expected {} {} {}", lhs, op, rhs);
      } else {
        message = fmt::format("comparison `{}` failed", op);
      }
    }
    return DecomposedExpression{.passed = passed, .message = std::move(message)};
  }
};

/**
 * Binds to an expression’s left-hand side via `operator<=>`, whose precedence sits between
 * arithmetic operators (tighter) and equality operators (looser), so a sub-expression like
 * `it - begin` binds as a single unit before capture, and `== 2` still applies afterward.
 */
struct ExpressionDecomposer {
  template<typename Lhs>
  ExpressionCapture<Lhs> operator<=>(const Lhs& lhs) const {
    return ExpressionCapture<Lhs>{.lhs = lhs};
  }
};

/** Finalizes the unary case, e.g. `THES_CHECK(some_flag)`, where no comparison operator ran. */
template<typename Lhs>
DecomposedExpression to_decomposed(const ExpressionCapture<Lhs>& capture) {
  const bool passed = static_cast<bool>(capture.lhs);
  std::string message{};
  if (!passed) {
    if constexpr (fmt::formattable<Lhs>) {
      message = fmt::format("expected {} to be truthy", capture.lhs);
    }
  }
  return DecomposedExpression{.passed = passed, .message = std::move(message)};
}

/**
 * Finalizes the binary case, where a comparison operator already produced a `DecomposedExpression`.
 */
inline DecomposedExpression to_decomposed(DecomposedExpression expression) {
  return expression;
}

//==================================================================================================
// Assertions
//==================================================================================================

/** Records a `DecomposedExpression` outcome against the currently running test case. */
inline void record(DecomposedExpression expression, std::string_view expression_text,
                   std::source_location location = std::source_location::current()) {
  Registry::instance().record({
    .passed = expression.passed,
    .expression = std::string{expression_text},
    .message = std::move(expression.message),
    .location = location,
  });
}

/** Returns whether invoking `fn` throws an exception convertible to `Exception`. */
template<typename Exception, typename Fn>
requires(std::invocable<Fn>)
bool throws_as(Fn&& fn) {
  try {
    std::forward<Fn>(fn)();
  } catch (const Exception&) {
    return true;
  } catch (...) {
    return false;
  }
  return false;
}

/** Returns whether invoking `fn` completes without throwing. */
template<typename Fn>
requires(std::invocable<Fn>)
bool does_not_throw(Fn&& fn) {
  try {
    std::forward<Fn>(fn)();
  } catch (...) {
    return false;
  }
  return true;
}
} // namespace thes::test

//--------------------------------------------------------------------------------------------------
// Macros
//--------------------------------------------------------------------------------------------------

#define THES_CONCAT_(a, b) a##b
#define THES_CONCAT(a, b) THES_CONCAT_(a, b)

#define THES_TEST_CASE(name, tags) \
  static void THES_CONCAT(thes_test_case_, __LINE__)(); \
  static const ::thes::test::AutoRegister THES_CONCAT(thes_test_reg_, __LINE__){ \
    [] noexcept { \
      using namespace std::literals::string_view_literals; \
      return name##sv; \
    }(), \
    [] noexcept { \
      using namespace std::literals::string_view_literals; \
      return tags##sv; \
    }(), \
    &THES_CONCAT(thes_test_case_, __LINE__)}; \
  static void THES_CONCAT(thes_test_case_, __LINE__)()

#define THES_CHECK(expr) \
  ::thes::test::record( \
    ::thes::test::to_decomposed(::thes::test::ExpressionDecomposer{} <=> expr /*NOLINT*/), #expr)

#define THES_CHECK_MESSAGE(expr, msg) \
  ::thes::test::record(::thes::test::DecomposedExpression{ \
    .passed = static_cast<bool>(expr), \
    .message = (msg), \
  })

#define THES_CHECK_THROWS_AS(expr, exception_type) \
  ::thes::test::record( \
    ::thes::test::DecomposedExpression{ \
      .passed = ::thes::test::throws_as<exception_type>([&] { (void)(expr); }), \
    }, \
    #expr " throws " #exception_type)

#define THES_CHECK_NOTHROW(expr) \
  ::thes::test::record( \
    ::thes::test::DecomposedExpression{ \
      .passed = ::thes::test::does_not_throw([&] { (void)(expr); }), \
    }, \
    #expr " does not throw")

#define THES_REQUIRE(expr) \
  ([&](std::source_location thes_location_) { \
    const auto thes_decomposed_ = \
      ::thes::test::to_decomposed(::thes::test::ExpressionDecomposer{} <=> expr /*NOLINT*/); \
    const bool thes_passed_ = thes_decomposed_.passed; \
    ::thes::test::record(thes_decomposed_, #expr, thes_location_); \
    if (!thes_passed_) { \
      throw ::thes::test::AssertionFailure{#expr}; \
    } \
  }(std::source_location::current()))

#define THES_TEST_MAIN() \
  int main() { \
    return ::thes::test::Registry::instance().run(); \
  }

#endif // INCLUDE_THESAUROS_TEST_TEST_HPP
