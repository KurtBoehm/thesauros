// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_ARGPARSE_GROUP_HPP
#define INCLUDE_THESAUROS_ARGPARSE_GROUP_HPP

#include <array>
#include <cassert>
#include <cstddef>
#include <string_view>
#include <type_traits>
#include <utility>

#include "thesauros/argparse/argument.hpp"
#include "thesauros/static-ranges/definitions/get-at.hpp"
#include "thesauros/static-ranges/definitions/static-apply.hpp"
#include "thesauros/static-ranges/sinks/to-tuple.hpp"
#include "thesauros/static-ranges/views/join.hpp"
#include "thesauros/types/tuple.hpp"

namespace thes::argparse {
/** Whether `T` is a `Tuple` of arguments, which is what a declaration flattens to. */
template<typename T>
inline constexpr bool is_argument_tuple = false;
template<AnyArgument... Args>
inline constexpr bool is_argument_tuple<Tuple<Args...>> = true;
/** A `Tuple` of arguments, which is what a declaration flattens to. */
template<typename T>
concept AnyArgumentTuple = is_argument_tuple<std::remove_cvref_t<T>>;

/**
 * Whether `T` may appear in a declaration: an argument, or anything declaring a sequence of them,
 * which is what an `ArgumentGroup` and an `ArgumentParser` are. The latter contributes the
 * arguments it declares rather than itself, so that declarations nest.
 */
template<typename T>
concept AnyArgumentSource = AnyArgument<T> || requires(const T& src) {
  { src.arguments() } -> AnyArgumentTuple;
};

namespace detail {
/** Whether the compile-time names of `Args...` are pairwise different. */
template<typename... Args>
inline constexpr bool names_are_unique = [] {
  const std::array<std::string_view, sizeof...(Args)> names{Args::name.view()...};
  for (std::size_t i = 0; i < sizeof...(Args); ++i) {
    for (std::size_t j = i + 1; j < sizeof...(Args); ++j) {
      if (names[i] == names[j]) {
        return false;
      }
    }
  }
  return true;
}();

/**
 * Whether no positional argument of `Args...` follows a remainder, which would never be reached
 * since a remainder collects every token that gets to it; this also limits a declaration to one
 * remainder. Unlike the same question for a list, this follows from the types alone.
 */
template<typename... Args>
inline constexpr bool remainder_is_last = [] {
  const std::array<bool, sizeof...(Args)> named{Args::is_named...};
  const std::array<bool, sizeof...(Args)> remainders{(Args::kind == ArgumentKind::remainder)...};
  bool ended = false;
  for (std::size_t i = 0; i < sizeof...(Args); ++i) {
    if (named[i]) {
      continue;
    }
    if (ended) {
      return false;
    }
    ended = remainders[i];
  }
  return true;
}();

/** Whether no two of `arguments` share a short or a long name, ignoring the ones without. */
template<typename... Args>
constexpr bool flags_are_unique(const Tuple<Args...>& arguments) {
  constexpr std::size_t argument_num = sizeof...(Args);
  const auto gather = [&](auto pick) {
    return star::static_apply<argument_num>([&]<std::size_t... I> {
      return std::array<std::string_view, argument_num>{pick(star::get_at<I>(arguments))...};
    });
  };
  const auto is_unique = [](const std::array<std::string_view, argument_num>& names) {
    for (std::size_t i = 0; i < argument_num; ++i) {
      for (std::size_t j = i + 1; j < argument_num; ++j) {
        if (!names[i].empty() && names[i] == names[j]) {
          return false;
        }
      }
    }
    return true;
  };
  return is_unique(gather([](const auto& arg) { return arg.short_name; })) &&
         is_unique(gather([](const auto& arg) { return arg.long_name; }));
}
} // namespace detail

/**
 * A bundle of argument declarations with no program of its own, so that a function can contribute
 * the arguments of one concern and the program can put the contributions together with `merge`:
 *
 *     constexpr auto logging_arguments() {
 *       return thes::argparse::ArgumentGroup{
 *         thes::argparse::counter<"verbose">("-v").help("Increase verbosity"),
 *         thes::argparse::flag<"quiet">("-q").help("Say nothing at all"),
 *       }.titled("Logging");
 *     }
 *
 * Arguments and groups may be mixed freely in the declaration, a group contributing the arguments
 * in it rather than itself, so that a contribution put together from others is written the same way
 * as one written out in a single place. Merging concatenates the declarations, so a group costs
 * nothing beyond the arguments in it.
 *
 * @tparam ArgTuple The `Tuple` of the arguments in the group, which is what a declaration flattens
 * to and thus need not be spelled out: `ArgumentGroup{…}` deduces it.
 */
template<typename ArgTuple>
struct ArgumentGroup;

namespace detail {
/**
 * The arguments `src` contributes to a flattened declaration: `src` itself if it is an argument,
 * and the arguments in it if it declares several, as a group and a parser do.
 */
template<AnyArgumentSource Src>
constexpr auto argument_tuple(const Src& src) {
  if constexpr (AnyArgument<Src>) {
    return Tuple<Src>{src};
  } else {
    return src.arguments();
  }
}

/** The arguments of `args`, which are arguments and groups thereof, in one flat tuple. */
template<AnyArgumentSource... ArgsOrGroups>
constexpr auto flatten_arguments(const ArgsOrGroups&... args) {
  if constexpr (sizeof...(ArgsOrGroups) == 0) {
    return Tuple<>{};
  } else {
    return star::to_tuple(star::joined(argument_tuple(args)...));
  }
}

/** The tuple the declaration `ArgsOrGroups` flattens to. */
template<AnyArgumentSource... ArgsOrGroups>
using FlattenedArguments = decltype(flatten_arguments(std::declval<const ArgsOrGroups&>()...));
} // namespace detail

template<AnyArgument... Args>
struct ArgumentGroup<Tuple<Args...>> {
  using Arguments = Tuple<Args...>;
  static constexpr std::size_t argument_num = sizeof...(Args);

  template<AnyArgumentSource... ArgsOrGroups>
  explicit constexpr ArgumentGroup(const ArgsOrGroups&... args)
      : arguments_{detail::flatten_arguments(args...)} {
    assert(detail::flags_are_unique(arguments_));
  }

  [[nodiscard]] constexpr const Arguments& arguments() const {
    return arguments_;
  }

  /** The group with `arg` appended. */
  template<AnyArgument Arg>
  [[nodiscard]] constexpr auto add(const Arg& arg) const {
    return appended(arg);
  }

  /** The group with the arguments of `groups` appended, in the order they are given in. */
  template<AnyArgumentSource... Groups>
  [[nodiscard]] constexpr auto merge(const Groups&... groups) const {
    return appended(groups...);
  }

  /**
   * A copy of the group whose named arguments are listed under the heading `title` in the help
   * text. Arguments that already belong to a section keep it, so that the innermost group a
   * contribution passed through decides where it is shown.
   */
  [[nodiscard]] constexpr ArgumentGroup titled(std::string_view title) const {
    ArgumentGroup copy = *this;
    star::static_apply<argument_num>([&]<std::size_t... I> {
      const auto fill = [title](auto& arg) {
        if (arg.section_name.empty()) {
          arg.section_name = title;
        }
      };
      (fill(star::get_at<I>(copy.arguments_)), ...);
    });
    return copy;
  }

private:
  static_assert(detail::names_are_unique<Args...>, "The argument names must be unique!");
  static_assert(detail::remainder_is_last<Args...>,
                "No positional argument may follow a remainder!");

  /** The group with the arguments of `args`, which may be arguments or groups, appended. */
  template<AnyArgumentSource... ArgsOrGroups>
  [[nodiscard]] constexpr auto appended(const ArgsOrGroups&... args) const {
    using Result = ArgumentGroup<detail::FlattenedArguments<ArgumentGroup, ArgsOrGroups...>>;
    return Result{*this, args...};
  }

  Arguments arguments_;
};

template<AnyArgumentSource... ArgsOrGroups>
ArgumentGroup(ArgsOrGroups...) -> ArgumentGroup<detail::FlattenedArguments<ArgsOrGroups...>>;
} // namespace thes::argparse

#endif // INCLUDE_THESAUROS_ARGPARSE_GROUP_HPP
