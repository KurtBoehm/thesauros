// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_ARGPARSE_ARGUMENT_HPP
#define INCLUDE_THESAUROS_ARGPARSE_ARGUMENT_HPP

#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <limits>
#include <optional>
#include <span>
#include <string_view>
#include <type_traits>
#include <utility>

#include "thesauros/argparse/value-parse.hpp"
#include "thesauros/string/static-string.hpp"
#include "thesauros/types/empty.hpp"
#include "thesauros/types/primitives.hpp"

namespace thes::argparse {
/**
 * The command-line tokens of a program (parameter names, argument values, etc.), excluding its
 * name.
 */
using TokenSpan = std::span<const char* const>;

/** The argument multiplicity representing “as many values as are given”. */
inline constexpr std::size_t unbounded = std::numeric_limits<std::size_t>::max();

/** How an argument is recognized and interpreted on the command line. */
enum struct ArgumentKind : u8 {
  /** Matched by its position among the tokens that are not named arguments. */
  positional,
  /** Like a positional argument, but collecting as many values as are given rather than one. */
  list,
  /** Introduced by a short or long name and followed by a value. */
  option,
  /** Introduced by a short or long name; either present or absent. */
  flag,
  /** Like a flag, but yielding the number of times it was specified. */
  counter,
  /** A trailing positional collecting every remaining token verbatim. */
  remainder,
};

/** Whether an argument must be given, may be absent, or falls back to a default. */
enum struct Presence : u8 {
  /** May be absent, in which cast it is an empty `std::optional`. */
  optional,
  /** Must be given, so that the value need not be wrapped in an `std::optional`. */
  required,
  /** May be absent, in which case the value is the argument’s default. */
  defaulted,
};

namespace detail {
/**
 * `Name` in upper case, which is the default placeholder for the value of an argument named `Name`.
 */
template<StaticString Name>
inline constexpr StaticString<Name.size> upper_name = [] {
  auto data = Name.data;
  for (char& c : data) {
    if ('a' <= c && c <= 'z') {
      c = static_cast<char>(c - 'a' + 'A');
    }
  }
  return StaticString<Name.size>{std::move(data)};
}();

/** `--` followed by `Name`, which is the default long name of a named argument named `Name`. */
template<StaticString Name>
inline constexpr auto default_long_name = StaticString{"--"} + Name;
} // namespace detail

/**
 * One command-line argument: its compile-time name and value type together with the associated
 * information. Instances are intended to be built by the functions `positional`, `option`, `flag`,
 * `counter`, and `remainder` and refined by the builder functions defined below, each of which
 * returns a modified copy rather than mutating in place, so that the whole declaration remains a
 * constant expression.
 *
 * @tparam Name The name which the parsed value is retrieved with.
 * @tparam T The type the value is converted to.
 * @tparam Kind How the argument is recognized and interpreted on the command line.
 * @tparam Pres Whether the argument must be given, may be absent, or has a default.
 * @tparam ChoiceNum The number of permitted values: zero if any value is permitted.
 */
template<StaticString Name, typename T, ArgumentKind Kind, Presence Pres, std::size_t ChoiceNum>
struct Argument {
  using Value = T;
  /** The type the parsed value is stored as: wrapped in an `std::optional` if it may be absent. */
  using Stored = std::conditional_t<Pres == Presence::optional, std::optional<T>, T>;
  using Choices = std::array<T, ChoiceNum>;
  /** The type of the default value: the empty type unless there is one to store. */
  using Default = std::conditional_t<Pres == Presence::defaulted, T, Empty>;
  /** The same argument with another presence and another number of choices. */
  template<Presence NewPres, std::size_t NewChoiceNum>
  using Rebound = Argument<Name, T, Kind, NewPres, NewChoiceNum>;

  static constexpr StaticString name = Name;
  static constexpr ArgumentKind kind = Kind;
  static constexpr Presence presence = Pres;
  static constexpr std::size_t choice_num = ChoiceNum;
  /** Whether the argument is introduced by a name rather than being positional. */
  static constexpr bool is_named =
    Kind == ArgumentKind::option || Kind == ArgumentKind::flag || Kind == ArgumentKind::counter;
  /** Whether the argument may consume a value that must be converted to a `T`. */
  static constexpr bool takes_value =
    Kind == ArgumentKind::option || Kind == ArgumentKind::positional;
  /** Whether the argument gathers several tokens rather than at most one. */
  static constexpr bool is_collecting =
    Kind == ArgumentKind::list || Kind == ArgumentKind::remainder;

  /** The name used to refer to the argument in messages. */
  [[nodiscard]] constexpr std::string_view display_name() const {
    if constexpr (is_named) {
      return long_name.empty() ? short_name : long_name;
    } else {
      return value_name;
    }
  }

  //------------------------------------------------------------------------------------------------
  // Builders
  //------------------------------------------------------------------------------------------------

  /** A copy of this argument using `text` in the help output. */
  [[nodiscard]] constexpr Argument help(std::string_view text) const {
    Argument copy = *this;
    copy.help_text = text;
    return copy;
  }

  /** A copy of this argument which is denoted by `text` in the help output. */
  [[nodiscard]] constexpr Argument metavar(std::string_view text) const {
    Argument copy = *this;
    copy.value_name = text;
    return copy;
  }

  /** A copy of this argument listed under the heading `title` in the help output. */
  [[nodiscard]] constexpr Argument section(std::string_view title) const {
    Argument copy = *this;
    copy.section_name = title;
    return copy;
  }

  /** A copy of this argument that must be specified on the command line. */
  [[nodiscard]] constexpr Rebound<Presence::required, ChoiceNum> required() const {
    return {short_name,   long_name, help_text, value_name,
            section_name, least_num, most_num,  choice_values};
  }

  /**
   * A copy of this argument that may be absent, in which case it yields an empty `std::optional`.
   */
  [[nodiscard]] constexpr Rebound<Presence::optional, ChoiceNum> optional() const {
    return {short_name,   long_name, help_text, value_name,
            section_name, least_num, most_num,  choice_values};
  }

  /**
   * A copy of this list collecting exactly `num` values. A list bounded this way need not be the
   * last positional argument, since it stops collecting once it is full.
   */
  [[nodiscard]] constexpr Argument exactly(std::size_t num) const
  requires(Kind == ArgumentKind::list)
  {
    return between(num, num);
  }

  /**
   * A copy of this list collecting between `least` and `most` values. Passing `unbounded` as `most`
   * leaves the upper end open, so that `between(2, thes::argparse::unbounded)` is “two or more”.
   */
  [[nodiscard]] constexpr Argument between(std::size_t least, std::size_t most) const
  requires(Kind == ArgumentKind::list)
  {
    assert(least <= most);
    Argument copy = *this;
    copy.least_num = least;
    copy.most_num = most;
    return copy;
  }

  /** A copy of this argument that takes on `value` when it is not given. */
  [[nodiscard]] constexpr Rebound<Presence::defaulted, ChoiceNum> default_to(T value) const {
    return {short_name, long_name, help_text,     value_name,      section_name,
            least_num,  most_num,  choice_values, std::move(value)};
  }

  /** A copy of this argument whose value must be one of `values`. */
  template<typename... Vs>
  requires(sizeof...(Vs) > 0 && (... && std::convertible_to<Vs, T>))
  [[nodiscard]] constexpr Rebound<Pres, sizeof...(Vs)> choices(const Vs&... values) const {
    using ReboundChoices = Rebound<Pres, sizeof...(Vs)>::Choices;
    return {short_name,   long_name, help_text, value_name,
            section_name, least_num, most_num,  ReboundChoices{static_cast<T>(values)...},
            default_value};
  }

  //------------------------------------------------------------------------------------------------
  // Public member variables
  //------------------------------------------------------------------------------------------------

  std::string_view short_name{};
  std::string_view long_name{};
  std::string_view help_text{};
  std::string_view value_name = detail::upper_name<Name>.view();
  /** The heading the argument is listed under: empty for the default section. */
  std::string_view section_name{};
  /** The fewest values a list must collect; meaningless for the other kinds. */
  std::size_t least_num{0};
  /** The most values a list may collect; meaningless for the other kinds. */
  std::size_t most_num{unbounded};
  Choices choice_values{};
  [[no_unique_address]] Default default_value{};
};

namespace detail {
/** A trait that detects any argument. */
template<typename T>
struct IsArgumentTrait : std::false_type {};
template<StaticString Name, typename T, ArgumentKind Kind, Presence Pres, std::size_t ChoiceNum>
struct IsArgumentTrait<Argument<Name, T, Kind, Pres, ChoiceNum>> : std::true_type {};

/**
 * Builds a named argument in whose names are specified by `name1` and `name2` if either is
 * non-empty; by default (i.e. both are empty), the long name is `--` followed by `Name` and there
 * is no short name. If non-empty, either name has to start with at least one hyphen: if it starts
 * with two, it is used as the long name, otherwise as the short name (in which case it can only
 * consist of a single letter). If both are long or both are short names, `name2` takes precedence.
 */
template<StaticString Name, typename T, ArgumentKind Kind, Presence Pres>
constexpr Argument<Name, T, Kind, Pres, 0> named_argument(std::string_view name1,
                                                          std::string_view name2) {
  Argument<Name, T, Kind, Pres, 0> arg{};
  arg.long_name = default_long_name<Name>.view();
  for (const std::string_view given : {name1, name2}) {
    if (given.empty()) {
      continue;
    }
    assert(given.size() > 1 && given.front() == '-');
    if (given.starts_with("--")) {
      arg.long_name = given;
    } else {
      assert(given.size() == 2);
      arg.short_name = given;
    }
  }
  return arg;
}
} // namespace detail

/** Whether `T` is any instantiation of `Argument`. */
template<typename T>
concept AnyArgument = detail::IsArgumentTrait<T>::value;

/**
 * A positional argument named `Name` holding a `T`, required unless followed by an `optional` or
 * `default_to`. Positional arguments are matched in declaration order.
 */
template<StaticString Name, ArgumentValue T = std::string_view>
[[nodiscard]] constexpr Argument<Name, T, ArgumentKind::positional, Presence::required, 0>
positional() {
  return {};
}

/**
 * An option named `Name` holding a `T`, which can be passed as `--name VALUE`, `--name=VALUE`,
 * `-n VALUE`, or `-nVALUE`. The two names may be given in any order (or omitted altogether) and are
 * told apart by the number of leading dashes; the previous example assumes they are `--name` and
 * `-n` (or vice versa). The long name defaults to `--` followed by `Name`; there is no short name
 * by default.
 */
template<StaticString Name, ArgumentValue T = std::string_view>
[[nodiscard]] constexpr auto option(std::string_view name1 = {}, std::string_view name2 = {}) {
  return detail::named_argument<Name, T, ArgumentKind::option, Presence::optional>(name1, name2);
}

/**
 * A Boolean flag named `Name`, which is `false` unless it is provided. See `option` for a
 * description of the names.
 */
template<StaticString Name>
[[nodiscard]] constexpr auto flag(std::string_view name1 = {}, std::string_view name2 = {}) {
  return detail::named_argument<Name, bool, ArgumentKind::flag, Presence::defaulted>(name1, name2);
}

/**
 * A counter named `Name`, which counts how often it was given: `-vvv` would yield a count of three.
 * See `option` for a description of the names.
 */
template<StaticString Name, std::unsigned_integral T = unsigned>
[[nodiscard]] constexpr auto counter(std::string_view name1 = {}, std::string_view name2 = {}) {
  return detail::named_argument<Name, T, ArgumentKind::counter, Presence::defaulted>(name1, name2);
}

/**
 * A trailing positional argument named `Name`, which collects as many values as are given rather
 * than one, i.e. “zero or more” values. Following it with `required` demands at least one, which is
 * “one or more”:
 *
 *     thes::argparse::list<"input-files">().required().help("The files to read.")
 *
 * Named arguments go on being recognized around the collected values, so that both `-o out a b` and
 * `a b -o out` leave `a` and `b` here. The values do have to arrive in one uninterrupted run: the
 * value is a `TokenSpan` referencing a sub-span of `argv`, keeping argument parsing free of memory
 * allocations, and a sub-span cannot leave out a token in the middle. A run broken up by a named
 * argument is reported as `ParseErrorKind::split_values`.
 */
template<StaticString Name>
[[nodiscard]] constexpr Argument<Name, TokenSpan, ArgumentKind::list, Presence::defaulted, 0>
list() {
  return {};
}

/**
 * A trailing positional argument named `Name`, which stops the command line from being interpreted
 * any further: the first token to reach it, and every token after that, is collected verbatim,
 * including tokens that would otherwise be named arguments. This is what a program wrapping another
 * one needs, so that `wrap --quiet cmd --flag` reads `--quiet` for itself and hands `cmd --flag` on
 * unchanged, the way `sudo`, `env`, `nice` and `timeout` do:
 *
 *     thes::argparse::remainder<"command">().required().help("The command to run.")
 *
 * Named arguments before the first collected token are still read, so a wrapper keeps its own
 * options; `--` also starts the collection, for a command line beginning with something that looks
 * like one. Use `list` instead where the values are just values and named arguments should go on
 * being recognized after them. The value is a `TokenSpan` referencing a sub-span of `argv`, keeping
 * argument parsing free of memory allocations.
 */
template<StaticString Name>
[[nodiscard]] constexpr Argument<Name, TokenSpan, ArgumentKind::remainder, Presence::defaulted, 0>
remainder() {
  return {};
}
} // namespace thes::argparse

#endif // INCLUDE_THESAUROS_ARGPARSE_ARGUMENT_HPP
