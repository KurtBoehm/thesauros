// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_ARGPARSE_ERROR_HPP
#define INCLUDE_THESAUROS_ARGPARSE_ERROR_HPP

#include <string_view>

#include "thesauros/types/primitives.hpp"

namespace thes::argparse {
/** Why a command line could not be turned into a set of argument values. */
enum struct ParseErrorKind : u8 {
  /** `-h`/`--help` was given, so the caller is expected to print the help text and stop. */
  help_requested,
  /** `--version` was given, so the caller is expected to print the version and stop. */
  version_requested,
  /** A token looks like a named argument, but no argument goes by that name. */
  unknown_argument,
  /** An argument that takes a value was given none. */
  missing_value,
  /** A flag or counter was given a value, as in `--verbose=1`. */
  unexpected_value,
  /** A value could not be converted to the type of its argument. */
  invalid_value,
  /** A value is not among the choices permitted for its argument. */
  invalid_choice,
  /** A required argument was not given. */
  missing_argument,
  /** More positional arguments were given than are declared. */
  excess_positional,
  /** The values of a collecting argument were interrupted by a named argument. */
  split_values,
};

/**
 * Describes why parsing failed. Every view points into `argv` or into a string literal, so a
 * `ParseError` owns nothing and stays valid for as long as the command line it describes.
 */
struct ParseError {
  ParseErrorKind kind{};
  /** The name of the offending argument, empty where no argument is implicated. */
  std::string_view argument{};
  /** The offending value, empty where the error concerns none. */
  std::string_view value{};

  /** Whether this is a request for the help text or the version rather than a failure. */
  [[nodiscard]] constexpr bool is_request() const {
    return kind == ParseErrorKind::help_requested || kind == ParseErrorKind::version_requested;
  }

  /** A description of `kind` alone, phrased to be followed by `argument` and `value`. */
  [[nodiscard]] constexpr std::string_view description() const {
    switch (kind) {
      case ParseErrorKind::help_requested: return "help requested";
      case ParseErrorKind::version_requested: return "version requested";
      case ParseErrorKind::unknown_argument: return "unknown argument";
      case ParseErrorKind::missing_value: return "missing value for argument";
      case ParseErrorKind::unexpected_value: return "unexpected value for argument";
      case ParseErrorKind::invalid_value: return "invalid value for argument";
      case ParseErrorKind::invalid_choice: return "impermissible value for argument";
      case ParseErrorKind::missing_argument: return "missing required argument";
      case ParseErrorKind::excess_positional: return "unexpected positional argument";
      case ParseErrorKind::split_values: return "values interrupted by a named argument for";
    }
    return "unknown error";
  }
};
} // namespace thes::argparse

#endif // INCLUDE_THESAUROS_ARGPARSE_ERROR_HPP
