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
#include <utility>

#include "thesauros/argparse/argument.hpp"
#include "thesauros/static-ranges/definitions/get-at.hpp"
#include "thesauros/static-ranges/definitions/static-apply.hpp"
#include "thesauros/types/tuple.hpp"

namespace thes::argparse {
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

/** Whether no two of `arguments` share a short or a long name, ignoring the ones without. */
template<typename... Args>
constexpr bool flags_are_unique(const Tuple<Args...>& arguments) {
  constexpr std::size_t argument_num = sizeof...(Args);
  const auto gather = [&](auto pick) {
    return star::static_apply<argument_num>([&]<std::size_t... Is>() {
      return std::array<std::string_view, argument_num>{pick(star::get_at<Is>(arguments))...};
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
 * Merging concatenates the declarations, so a group costs nothing beyond the arguments in it and
 * the parser it ends up in is the same as one written out in a single place.
 */
template<typename... Args>
struct ArgumentGroup {
  using Arguments = Tuple<Args...>;
  static constexpr std::size_t argument_num = sizeof...(Args);

  explicit constexpr ArgumentGroup(Args... args) : arguments_{std::move(args)...} {
    assert(detail::flags_are_unique(arguments_));
  }

  [[nodiscard]] constexpr const Arguments& arguments() const {
    return arguments_;
  }

  /** The group with `arg` appended. */
  template<typename Arg>
  [[nodiscard]] constexpr ArgumentGroup<Args..., Arg> add(Arg arg) const {
    return star::static_apply<argument_num>([&]<std::size_t... Is>() {
      return ArgumentGroup<Args..., Arg>{star::get_at<Is>(arguments_)..., std::move(arg)};
    });
  }

  /** The group with the arguments of `groups` appended, in the order they are given in. */
  template<typename First, typename... Rest>
  [[nodiscard]] constexpr auto merge(const First& first, const Rest&... rest) const {
    if constexpr (sizeof...(Rest) == 0) {
      return merged(first);
    } else {
      return merged(first).merge(rest...);
    }
  }

  /**
   * A copy of the group whose named arguments are listed under the heading `title` in the help
   * text. Arguments that already belong to a section keep it, so that the innermost group a
   * contribution passed through decides where it is shown.
   */
  [[nodiscard]] constexpr ArgumentGroup titled(std::string_view title) const {
    ArgumentGroup copy = *this;
    star::static_apply<argument_num>([&]<std::size_t... Is>() {
      const auto fill = [title](auto& arg) {
        if (arg.section_name.empty()) {
          arg.section_name = title;
        }
      };
      (fill(star::get_at<Is>(copy.arguments_)), ...);
    });
    return copy;
  }

private:
  static_assert(detail::names_are_unique<Args...>, "The argument names must be unique!");

  /** The group with the arguments of `other` appended. */
  template<typename... Others>
  [[nodiscard]] constexpr ArgumentGroup<Args..., Others...>
  merged(const ArgumentGroup<Others...>& other) const {
    return star::static_apply<argument_num>([&]<std::size_t... Is>() {
      return star::static_apply<sizeof...(Others)>([&]<std::size_t... Js>() {
        return ArgumentGroup<Args..., Others...>{star::get_at<Is>(arguments_)...,
                                                 star::get_at<Js>(other.arguments())...};
      });
    });
  }

  Arguments arguments_;
};

template<typename... Args>
ArgumentGroup(Args...) -> ArgumentGroup<Args...>;
} // namespace thes::argparse

#endif // INCLUDE_THESAUROS_ARGPARSE_GROUP_HPP
