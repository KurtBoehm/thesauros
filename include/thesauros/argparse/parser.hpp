// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_ARGPARSE_PARSER_HPP
#define INCLUDE_THESAUROS_ARGPARSE_PARSER_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <expected>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

#include "thesauros/argparse/argument.hpp"
#include "thesauros/argparse/error.hpp"
#include "thesauros/argparse/group.hpp"
#include "thesauros/argparse/help.hpp"
#include "thesauros/argparse/value-parse.hpp"
#include "thesauros/static-ranges/definitions/get-at.hpp"
#include "thesauros/static-ranges/definitions/static-apply.hpp"
#include "thesauros/string/static-string.hpp"
#include "thesauros/types/tuple.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes::argparse {
/** The program metadata shown in the help text. */
struct ProgramInfo {
  /** The program name; if empty, `argv[0]` is used. */
  std::string_view name{};
  /** The version shown for `--version`; if empty, `--version` is not recognized. */
  std::string_view version{};
  /** A description shown above the usage line. */
  std::string_view description{};
  /** A closing remark shown below the argument list. */
  std::string_view epilog{};
};

/**
 * The values a successful parse produced, retrieved by the compile-time names of the arguments that
 * produced them: `values.get<"count">()`. The type of each value follows from its declaration, so
 * no type is named twice and nothing is looked up at run time.
 */
template<typename... Args>
struct ArgumentValues {
  using Storage = Tuple<typename Args::Stored...>;
  static constexpr std::size_t size = sizeof...(Args);

  /** The index of the argument named `Name`, or `size` if no argument goes by that name. */
  template<StaticString Name>
  static constexpr std::size_t index_of = [] {
    const std::array<bool, size> matches{(Args::name == Name)...};
    for (std::size_t i = 0; i < size; ++i) {
      if (matches[i]) {
        return i;
      }
    }
    return size;
  }();

  /** Whether an argument named `Name` is declared. */
  template<StaticString Name>
  static constexpr bool contains = index_of<Name> < size;

  /** The value of the argument named `Name`. */
  template<StaticString Name>
  [[nodiscard]] constexpr decltype(auto) get(this auto&& self) {
    static_assert(contains<Name>, "There is no argument of this name!");
    return star::get_at<index_of<Name>>(std::forward<decltype(self)>(self).storage);
  }

  // Public so that the parser can fill the values in one by one.
  Storage storage{};
};

/**
 * A command-line parser declaring its arguments up front, in the vein of p-ranav/argparse but
 * resolved at compile time: the argument names are template parameters, the values live in a tuple
 * whose types follow from the declarations, and parsing writes nothing but views into `argv`. No
 * step of a parse allocates.
 *
 * The parser handles `-h`/`--help`, `--version` if a version is set, `--` to end the named
 * arguments, `--name=value`, `-nvalue` and the bundling of short flags as in `-xvf`. A token
 * starting with a dash followed by a digit or a dot is a value, so that negative numbers need no
 * escaping.
 *
 *     constexpr thes::argparse::ArgumentParser parser{
 *       thes::argparse::ProgramInfo{.name = "demo", .version = "1.0"},
 *       thes::argparse::positional<"input">().help("The file to read"),
 *       thes::argparse::option<"count", int>("-c").default_to(1).help("How often to read it"),
 *       thes::argparse::counter<"verbose">("-v").help("Increase verbosity"),
 *     };
 *     const auto values = parser.parse_or_exit(argc, argv);
 *     const int count = values.get<"count">();
 *
 * The arguments need not be declared in one place: a function can return an `ArgumentGroup`
 * covering one concern, and `merge` puts several such contributions together into one parser.
 */
template<typename... Args>
struct ArgumentParser {
  using Values = ArgumentValues<Args...>;
  using Arguments = Tuple<Args...>;

  static constexpr std::size_t argument_num = sizeof...(Args);
  /** The index standing for “no such argument”. */
  static constexpr std::size_t no_index = argument_num;

  explicit constexpr ArgumentParser(ProgramInfo program_info, Args... args)
      : info_{program_info}, arguments_{std::move(args)...} {
    assert(detail::flags_are_unique(arguments_));
    assert(unbounded_is_last());
  }

  //------------------------------------------------------------------------------------------------
  // Declaration
  //------------------------------------------------------------------------------------------------

  [[nodiscard]] constexpr const ProgramInfo& info() const {
    return info_;
  }
  [[nodiscard]] constexpr const Arguments& arguments() const {
    return arguments_;
  }

  /**
   * Whether no positional argument that collects without an upper bound is followed by another one,
   * which would never be reached. A parser declared `constexpr` has this checked when it is built.
   */
  [[nodiscard]] constexpr bool unbounded_is_last() const {
    for (std::size_t i = 0; i + 1 < positional_num; ++i) {
      const std::size_t index = positional_indices[i];
      if (collecting_mask[index] && most_num_at(index) == unbounded) {
        return false;
      }
    }
    return true;
  }

  /** A parser with `arg` appended to its arguments. */
  template<typename Arg>
  [[nodiscard]] constexpr ArgumentParser<Args..., Arg> add(Arg arg) const {
    return star::static_apply<argument_num>([&]<std::size_t... Is>() {
      return ArgumentParser<Args..., Arg>{info_, star::get_at<Is>(arguments_)..., std::move(arg)};
    });
  }

  /**
   * A parser with the arguments of `groups` appended, in the order they are given in, which is how
   * the contributions of several functions are put together.
   */
  template<typename First, typename... Rest>
  [[nodiscard]] constexpr auto merge(const First& first, const Rest&... rest) const {
    if constexpr (sizeof...(Rest) == 0) {
      return merged(first);
    } else {
      return merged(first).merge(rest...);
    }
  }

  //------------------------------------------------------------------------------------------------
  // Parsing
  //------------------------------------------------------------------------------------------------

  /** Parses `tokens`, which must not contain the program name. */
  [[nodiscard]] constexpr std::expected<Values, ParseError> parse(TokenSpan tokens) const {
    Values values{};
    std::array<bool, argument_num> seen{};
    std::size_t positional_index = 0;
    Collected collected{};
    bool named_ended = false;

    for (std::size_t i = 0; i < tokens.size(); ++i) {
      const std::string_view token{tokens[i]};

      if (!named_ended && token == "--") {
        named_ended = true;
        continue;
      }

      const std::optional<ParseError> error = [&] {
        if (!named_ended && is_named_token(token)) {
          return token.starts_with("--") ? parse_long(token, tokens, i, values, seen)
                                         : parse_short(token, tokens, i, values, seen);
        }
        return parse_positional(token, tokens, i, positional_index, collected, values, seen);
      }();
      if (error.has_value()) {
        return std::unexpected{*error};
      }
    }

    const std::optional<ParseError> error = complete(collected, values, seen);
    if (error.has_value()) {
      return std::unexpected{*error};
    }
    return values;
  }

  /** Parses `argv[1]` through `argv[argc - 1]`, ignoring the program name in `argv[0]`. */
  [[nodiscard]] constexpr std::expected<Values, ParseError> parse(int argc,
                                                                  const char* const* argv) const {
    if (argc < 2) {
      return parse(TokenSpan{});
    }
    return parse(TokenSpan{argv + 1, static_cast<std::size_t>(argc - 1)});
  }

  /**
   * Parses `argv` and returns the values, or ends the program: with status 0 after writing the help
   * text or the version to standard output, and with status 2 after writing the error and the usage
   * line to standard error.
   */
  [[nodiscard]] Values parse_or_exit(int argc, const char* const* argv) const {
    const std::string_view program = (argc > 0) ? std::string_view{argv[0]} : std::string_view{};
    std::expected<Values, ParseError> result = parse(argc, argv);
    if (result.has_value()) {
      return std::move(*result);
    }

    const ParseError error = result.error();
    if (error.kind == ParseErrorKind::help_requested) {
      print_help(stdout, program);
      std::exit(EXIT_SUCCESS); // NOLINT
    }
    if (error.kind == ParseErrorKind::version_requested) {
      detail::write_text(stdout, info_.version);
      detail::write_text(stdout, "\n");
      std::exit(EXIT_SUCCESS); // NOLINT
    }
    print_error(error, stderr, program);
    std::exit(EXIT_FAILURE); // NOLINT
  }

  //------------------------------------------------------------------------------------------------
  // Help output
  //------------------------------------------------------------------------------------------------

  /** Writes the usage line to `out`, naming the program `program` unless `info().name` is set. */
  void print_usage(std::FILE* out, std::string_view program = {}) const {
    const detail::TextWriter sink{out};
    sink("Usage: ");
    sink(program_name(program));
    if (!has_own_help()) {
      sink(" [-h]");
    }
    if (!info_.version.empty()) {
      sink(" [--version]");
    }
    for_each_argument([&](const auto& arg) {
      if constexpr (std::decay_t<decltype(arg)>::is_named) {
        sink(" ");
        detail::usage_parts(arg, sink);
      }
    });
    for_each_argument([&](const auto& arg) {
      if constexpr (!std::decay_t<decltype(arg)>::is_named) {
        sink(" ");
        detail::usage_parts(arg, sink);
      }
    });
    sink("\n");
  }

  /** Writes the description, the usage line, the argument list and the epilog to `out`. */
  void print_help(std::FILE* out = stdout, std::string_view program = {}) const {
    const detail::TextWriter sink{out};
    if (!info_.description.empty()) {
      sink(info_.description);
      sink("\n\n");
    }
    print_usage(out, program);

    const std::size_t width = list_width();
    if constexpr (positional_num > 0) {
      sink("\nPositional arguments:\n");
      for_each_argument([&](const auto& arg) {
        if constexpr (!std::decay_t<decltype(arg)>::is_named) {
          detail::write_row(out, arg, width);
        }
      });
    }

    const auto write_section = [&](std::string_view title) {
      for_each_argument([&](const auto& arg) {
        if constexpr (std::decay_t<decltype(arg)>::is_named) {
          if (arg.section_name == title) {
            detail::write_row(out, arg, width);
          }
        }
      });
    };

    // The arguments belonging to no group of their own head the list, together with the two the
    // parser provides by itself.
    const bool implicit_shown = !has_own_help() || !info_.version.empty();
    if (implicit_shown || named_num_in({}) > 0) {
      sink("\nOptions:\n");
      if (!has_own_help()) {
        detail::write_plain_row(out, help_label, width, "Show this help text and exit");
      }
      if (!info_.version.empty()) {
        detail::write_plain_row(out, version_label, width, "Show the version and exit");
      }
      write_section({});
    }

    const auto [sections, section_num] = section_names();
    for (std::size_t i = 0; i < section_num; ++i) {
      sink("\n");
      sink(sections[i]);
      sink(":\n");
      write_section(sections[i]);
    }

    if (!info_.epilog.empty()) {
      sink("\n");
      sink(info_.epilog);
      sink("\n");
    }
  }

  /** Writes `error` and the usage line to `out`. */
  void print_error(const ParseError& error, std::FILE* out = stderr,
                   std::string_view program = {}) const {
    const detail::TextWriter sink{out};
    sink(program_name(program));
    sink(": error: ");
    sink(error.description());
    if (!error.argument.empty()) {
      sink(" “");
      sink(error.argument);
      sink("”");
    }
    if (!error.value.empty()) {
      sink(": “");
      sink(error.value);
      sink("”");
    }
    sink("\n");
    print_usage(out, program);
  }

private:
  static constexpr std::string_view help_label = "-h, --help";
  static constexpr std::string_view version_label = "    --version";

  static constexpr std::array<bool, argument_num> positional_mask{!Args::is_named...};
  static constexpr std::array<bool, argument_num> remainder_mask{
    (Args::kind == ArgumentKind::remainder)...};
  static constexpr std::array<bool, argument_num> collecting_mask{Args::is_collecting...};
  static constexpr std::array<bool, argument_num> list_mask{(Args::kind == ArgumentKind::list)...};
  static constexpr std::array<bool, argument_num> value_mask{Args::takes_value...};
  static constexpr std::size_t positional_num =
    static_cast<std::size_t>(std::ranges::count(positional_mask, true));
  static constexpr std::array<std::size_t, positional_num> positional_indices = [] {
    std::array<std::size_t, positional_num> indices{};
    std::size_t found = 0;
    for (std::size_t i = 0; i < argument_num; ++i) {
      if (positional_mask[i]) {
        indices[found++] = i;
      }
    }
    return indices;
  }();

  static_assert(detail::names_are_unique<Args...>, "The argument names must be unique!");

  /** The parser with the arguments of `group` appended. */
  template<typename... Others>
  [[nodiscard]] constexpr ArgumentParser<Args..., Others...>
  merged(const ArgumentGroup<Others...>& group) const {
    return star::static_apply<argument_num>([&]<std::size_t... Is>() {
      return star::static_apply<sizeof...(Others)>([&]<std::size_t... Js>() {
        return ArgumentParser<Args..., Others...>{info_, star::get_at<Is>(arguments_)...,
                                                  star::get_at<Js>(group.arguments())...};
      });
    });
  }

  /** Whether `token` introduces one or more named arguments rather than being a value. */
  static constexpr bool is_named_token(std::string_view token) {
    // A lone dash, and anything like `-1` or `-.5`, is a value rather than a name.
    return token.size() > 1 && token.front() == '-' && (token[1] < '0' || token[1] > '9') &&
           token[1] != '.';
  }

  /** The most values the argument at `index` may collect. */
  [[nodiscard]] constexpr std::size_t most_num_at(std::size_t index) const {
    std::size_t most = unbounded;
    star::static_apply<argument_num>([&]<std::size_t... Is>() {
      static_cast<void>(
        ((Is == index ? (most = star::get_at<Is>(arguments_).most_num, true) : false) || ...));
    });
    return most;
  }

  /** The half-open range of tokens a list has collected so far. */
  struct Collected {
    std::size_t begin{};
    std::size_t end{};
  };

  /** Invokes `op` with the argument whose compile-time index matches the run-time index `index`. */
  template<typename Op>
  static constexpr std::optional<ParseError> visit_argument(std::size_t index, Op op) {
    std::optional<ParseError> error{};
    star::static_apply<argument_num>([&]<std::size_t... Is>() {
      static_cast<void>(((Is == index && (error = op(index_tag<Is>), true)) || ...));
    });
    return error;
  }

  /** Invokes `op` with each argument in declaration order. */
  template<typename Op>
  constexpr void for_each_argument(Op op) const {
    star::static_apply<argument_num>(
      [&]<std::size_t... Is>() { (op(star::get_at<Is>(arguments_)), ...); });
  }

  /** The index of the first argument satisfying `pred`, or `no_index`. */
  template<typename Pred>
  constexpr std::size_t find_argument(Pred pred) const {
    std::size_t found = no_index;
    star::static_apply<argument_num>([&]<std::size_t... Is>() {
      static_cast<void>(((pred(star::get_at<Is>(arguments_)) ? (found = Is, true) : false) || ...));
    });
    return found;
  }

  /** The index of the argument whose long name is `name`, or `no_index`. */
  [[nodiscard]] constexpr std::size_t find_long(std::string_view name) const {
    return find_argument(
      [name](const auto& arg) { return !arg.long_name.empty() && arg.long_name == name; });
  }

  /** The index of the argument whose short name is `letter` preceded by a dash, or `no_index`. */
  [[nodiscard]] constexpr std::size_t find_short(char letter) const {
    return find_argument([letter](const auto& arg) {
      return arg.short_name.size() == 2 && arg.short_name[1] == letter;
    });
  }

  /** Whether the caller declared an argument of their own for `-h` or `--help`. */
  [[nodiscard]] constexpr bool has_own_help() const {
    return find_long("--help") != no_index || find_short('h') != no_index;
  }

  /** The name to show for the program: the configured one, or `fallback`. */
  [[nodiscard]] constexpr std::string_view program_name(std::string_view fallback) const {
    return info_.name.empty() ? fallback : info_.name;
  }

  /** The number of named arguments listed under the heading `title`. */
  [[nodiscard]] constexpr std::size_t named_num_in(std::string_view title) const {
    std::size_t found = 0;
    for_each_argument([&](const auto& arg) {
      if constexpr (std::decay_t<decltype(arg)>::is_named) {
        if (arg.section_name == title) {
          ++found;
        }
      }
    });
    return found;
  }

  /** The headings of the named arguments in order of first appearance, and how many there are. */
  [[nodiscard]] constexpr Tuple<std::array<std::string_view, argument_num>, std::size_t>
  section_names() const {
    using Result = Tuple<std::array<std::string_view, argument_num>, std::size_t>;
    std::array<std::string_view, argument_num> titles{};
    std::size_t title_num = 0;
    for_each_argument([&](const auto& arg) {
      if constexpr (std::decay_t<decltype(arg)>::is_named) {
        if (arg.section_name.empty()) {
          return;
        }
        for (std::size_t i = 0; i < title_num; ++i) {
          if (titles[i] == arg.section_name) {
            return;
          }
        }
        titles[title_num++] = arg.section_name;
      }
    });
    return Result{titles, title_num};
  }

  /** The width the labels in the argument list are padded to. */
  [[nodiscard]] std::size_t list_width() const {
    std::size_t width = 0;
    if (!has_own_help()) {
      width = std::max(width, help_label.size());
    }
    if (!info_.version.empty()) {
      width = std::max(width, version_label.size());
    }
    for_each_argument([&](const auto& arg) { width = std::max(width, detail::label_width(arg)); });
    return width;
  }

  /** Converts `text` and stores it in `slot`, rejecting values outside the argument’s choices. */
  template<typename Arg, typename Slot>
  static constexpr std::optional<ParseError> assign(const Arg& arg, Slot& slot,
                                                    std::string_view text) {
    const std::optional<typename Arg::Value> value = parse_value<typename Arg::Value>(text);
    if (!value.has_value()) {
      return ParseError{ParseErrorKind::invalid_value, arg.display_name(), text};
    }
    if constexpr (Arg::choice_num > 0) {
      if (std::ranges::find(arg.choice_values, *value) == arg.choice_values.end()) {
        return ParseError{ParseErrorKind::invalid_choice, arg.display_name(), text};
      }
    }
    slot = *value;
    return std::nullopt;
  }

  /**
   * Applies the named argument at `index`, taking its value from `inline_value` or, failing that,
   * from the token after `token_index`, which is advanced past it.
   */
  constexpr std::optional<ParseError> apply_named(std::size_t index,
                                                  std::optional<std::string_view> inline_value,
                                                  TokenSpan tokens, std::size_t& token_index,
                                                  Values& values,
                                                  std::array<bool, argument_num>& seen) const {
    return visit_argument(index, [&]<std::size_t I>(IndexTag<I> /*tag*/) {
      using Arg = std::decay_t<decltype(star::get_at<I>(arguments_))>;
      const Arg& arg = star::get_at<I>(arguments_);
      auto& slot = star::get_at<I>(values.storage);
      seen[I] = true;

      if constexpr (Arg::kind == ArgumentKind::flag || Arg::kind == ArgumentKind::counter) {
        if (inline_value.has_value()) {
          return std::optional{
            ParseError{ParseErrorKind::unexpected_value, arg.display_name(), *inline_value}};
        }
        if constexpr (Arg::kind == ArgumentKind::flag) {
          slot = true;
        } else {
          ++slot;
        }
        return std::optional<ParseError>{};
      } else if constexpr (Arg::kind == ArgumentKind::option) {
        std::string_view text{};
        if (inline_value.has_value()) {
          text = *inline_value;
        } else if (token_index + 1 < tokens.size()) {
          text = std::string_view{tokens[++token_index]};
        } else {
          return std::optional{ParseError{ParseErrorKind::missing_value, arg.display_name()}};
        }
        return assign(arg, slot, text);
      } else {
        // Positional arguments are never reached by name, but the branch has to compile.
        return std::optional{ParseError{ParseErrorKind::unknown_argument, arg.display_name()}};
      }
    });
  }

  /** Applies a token of the form `--name`, `--name=value` or `--name value`. */
  constexpr std::optional<ParseError> parse_long(std::string_view token, TokenSpan tokens,
                                                 std::size_t& token_index, Values& values,
                                                 std::array<bool, argument_num>& seen) const {
    std::string_view name = token;
    std::optional<std::string_view> inline_value{};
    if (const std::size_t equals = token.find('='); equals != std::string_view::npos) {
      name = token.substr(0, equals);
      inline_value = token.substr(equals + 1);
    }

    const std::size_t index = find_long(name);
    if (index == no_index) {
      if (name == "--help") {
        return ParseError{.kind = ParseErrorKind::help_requested};
      }
      if (name == "--version" && !info_.version.empty()) {
        return ParseError{.kind = ParseErrorKind::version_requested};
      }
      return ParseError{.kind = ParseErrorKind::unknown_argument, .argument = name};
    }
    return apply_named(index, inline_value, tokens, token_index, values, seen);
  }

  /** Applies a token of the form `-abc`, where only the last letter may take a value. */
  constexpr std::optional<ParseError> parse_short(std::string_view token, TokenSpan tokens,
                                                  std::size_t& token_index, Values& values,
                                                  std::array<bool, argument_num>& seen) const {
    std::string_view rest = token.substr(1);
    while (!rest.empty()) {
      const char letter = rest.front();
      rest.remove_prefix(1);

      const std::size_t index = find_short(letter);
      if (index == no_index) {
        if (letter == 'h' && !has_own_help()) {
          return ParseError{.kind = ParseErrorKind::help_requested};
        }
        return ParseError{.kind = ParseErrorKind::unknown_argument, .argument = token};
      }

      std::optional<std::string_view> inline_value{};
      const bool takes_value = value_mask[index];
      if (takes_value && !rest.empty()) {
        if (rest.front() == '=') {
          rest.remove_prefix(1);
        }
        inline_value = rest;
        rest = {};
      }

      const std::optional<ParseError> error =
        apply_named(index, inline_value, tokens, token_index, values, seen);
      if (error.has_value()) {
        return error;
      }
      if (takes_value) {
        break;
      }
    }
    return std::nullopt;
  }

  /** Applies a token that is no named argument to the next positional argument. */
  constexpr std::optional<ParseError> parse_positional(std::string_view token, TokenSpan tokens,
                                                       std::size_t& token_index,
                                                       std::size_t& positional_index,
                                                       Collected& collected, Values& values,
                                                       std::array<bool, argument_num>& seen) const {
    if (positional_index < positional_num) {
      const std::size_t current = positional_indices[positional_index];
      if (list_mask[current] && seen[current] &&
          collected.end - collected.begin >= most_num_at(current)) {
        ++positional_index;
      }
    }
    if (positional_index >= positional_num) {
      return ParseError{ParseErrorKind::excess_positional, token};
    }
    const std::size_t index = positional_indices[positional_index];
    const std::size_t first = token_index;

    const std::optional<ParseError> error = visit_argument(index, [&]<std::size_t I>(
                                                                    IndexTag<I> /*tag*/) {
      using Arg = std::decay_t<decltype(star::get_at<I>(arguments_))>;
      const Arg& arg = star::get_at<I>(arguments_);
      auto& slot = star::get_at<I>(values.storage);

      if constexpr (Arg::kind == ArgumentKind::remainder) {
        // Reaching a remainder ends the interpretation of the command line: the rest of it is
        // this argument’s, whatever it looks like.
        seen[I] = true;
        slot = tokens.subspan(first);
        return std::optional<ParseError>{};
      } else if constexpr (Arg::kind == ArgumentKind::list) {
        if (!seen[I]) {
          collected.begin = first;
        } else if (first != collected.end) {
          // The run has already been closed by a named argument in between, and a sub-span
          // cannot leave that one out again.
          return std::optional{ParseError{ParseErrorKind::split_values, arg.display_name(), token}};
        }
        collected.end = first + 1;
        seen[I] = true;
        slot = tokens.subspan(collected.begin, collected.end - collected.begin);
        return std::optional<ParseError>{};
      } else if constexpr (Arg::kind == ArgumentKind::positional) {
        seen[I] = true;
        return assign(arg, slot, token);
      } else {
        // Named arguments are never reached by position, but the branch has to compile.
        return std::optional{ParseError{ParseErrorKind::excess_positional, token}};
      }
    });
    if (error.has_value()) {
      return error;
    }

    if (remainder_mask[index]) {
      // Everything the remainder swallowed is consumed, so the loop is done.
      token_index = tokens.size();
    } else if (!collecting_mask[index]) {
      // A list stays the current positional argument, so that it goes on collecting.
      ++positional_index;
    }
    return std::nullopt;
  }

  /** Fills in the defaults of the arguments that were not given and reports the missing ones. */
  constexpr std::optional<ParseError> complete(const Collected& collected, Values& values,
                                               const std::array<bool, argument_num>& seen) const {
    const auto finish = [&]<std::size_t I>(IndexTag<I> /*tag*/) {
      using Arg = std::decay_t<decltype(star::get_at<I>(arguments_))>;
      [[maybe_unused]] const Arg& arg = star::get_at<I>(arguments_);

      if constexpr (Arg::kind == ArgumentKind::list) {
        // Being required is the same as having to collect at least one value, so the two bounds
        // are one and the same question.
        const std::size_t got = seen[I] ? collected.end - collected.begin : 0;
        const std::size_t least =
          std::max(arg.least_num, std::size_t{Arg::presence == Presence::required ? 1 : 0});
        if (got < least) {
          return std::optional{ParseError{ParseErrorKind::missing_argument, arg.display_name()}};
        }
      }
      if (seen[I]) {
        return std::optional<ParseError>{};
      }
      if constexpr (Arg::presence == Presence::required) {
        return std::optional{ParseError{ParseErrorKind::missing_argument, arg.display_name()}};
      } else if constexpr (Arg::presence == Presence::defaulted) {
        star::get_at<I>(values.storage) = arg.default_value;
        return std::optional<ParseError>{};
      } else {
        return std::optional<ParseError>{};
      }
    };

    std::optional<ParseError> error{};
    star::static_apply<argument_num>([&]<std::size_t... Is>() {
      static_cast<void>(((error = finish(index_tag<Is>), error.has_value()) || ...));
    });
    return error;
  }

  ProgramInfo info_;
  Arguments arguments_;
};

template<typename... Args>
ArgumentParser(ProgramInfo, Args...) -> ArgumentParser<Args...>;
} // namespace thes::argparse

#endif // INCLUDE_THESAUROS_ARGPARSE_PARSER_HPP
