// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_ARGPARSE_HELP_HPP
#define INCLUDE_THESAUROS_ARGPARSE_HELP_HPP

#include <concepts>
#include <cstddef>
#include <cstdio>
#include <span>
#include <string_view>

#include "thesauros/argparse/argument.hpp"
#include "thesauros/charconv/numeric-string.hpp"
#include "thesauros/concepts/type-traits.hpp"
#include "thesauros/types/type-tag.hpp"

namespace thes::argparse {
namespace detail {
/** Writes `text` to `out`. */
inline void write_text(std::FILE* out, std::string_view text) {
  static_cast<void>(std::fwrite(text.data(), 1, text.size(), out));
}

/** Writes `count` spaces to `out`. */
inline void write_spaces(std::FILE* out, std::size_t count) {
  for (std::size_t i = 0; i < count; ++i) {
    static_cast<void>(std::fputc(' ', out));
  }
}

/**
 * A sink writing the fragments it is handed to `out`.
 * The help text is assembled from fragments handed to a sink so that measuring a line and writing
 * it share one description of what the line contains.
 */
struct TextWriter {
  std::FILE* out;

  void operator()(std::string_view text) const {
    write_text(out, text);
  }
};

/** A sink adding up the lengths of the fragments it is handed. */
struct TextCounter {
  std::size_t& total;

  void operator()(std::string_view text) const {
    total += text.size();
  }
};

/**
 * Whether `argument_value_text` is defined for `T` and findable by argument-dependent lookup, which
 * is how a type that is neither a string nor a number says how its values are written.
 */
template<typename T>
concept HasValueText = requires(const T& value) {
  { argument_value_text(thes::type_tag<T>, value) } -> std::convertible_to<std::string_view>;
};

/**
 * Whether `argument_value_names` is defined for `T` and findable by argument-dependent lookup, the
 * way a type admitting only a fixed set of values, such as an enumeration, lists them.
 */
template<typename T>
concept HasValueNames = requires {
  {
    argument_value_names(thes::type_tag<T>)
  } -> std::convertible_to<std::span<const std::string_view>>;
};

/** Whether values of type `T` can be shown in the help text. */
template<typename T>
concept Printable = HasValueText<T> || std::same_as<T, bool> ||
                    std::convertible_to<T, std::string_view> || Numeric<T>;

/** Hands `value` to `sink` in the form it would take on the command line. */
template<Printable T>
void write_value(const auto& sink, const T& value) {
  if constexpr (HasValueText<T>) {
    sink(std::string_view{argument_value_text(thes::type_tag<T>, value)});
  } else if constexpr (std::same_as<T, bool>) {
    sink(value ? "true" : "false");
  } else if constexpr (std::convertible_to<T, std::string_view>) {
    sink(std::string_view{value});
  } else {
    const auto text = numeric_string(value);
    if (text.has_value()) {
      sink(std::string_view{*text});
    }
  }
}

/** Hands a “(choices: …)” tail listing `values` to `sink`. */
void write_choices(const auto& sink, const auto& values) {
  sink(" (choices: ");
  bool first = true;
  for (const auto& value : values) {
    if (!first) {
      sink(", ");
    }
    first = false;
    write_value(sink, value);
  }
  sink(")");
}

/** Hands the fragments of the argument’s entry in the argument list to `sink`, in order. */
template<typename Arg>
void label_parts(const Arg& arg, const auto& sink) {
  if constexpr (Arg::is_named) {
    if (arg.short_name.empty()) {
      sink("    ");
    } else {
      sink(arg.short_name);
      sink(arg.long_name.empty() ? "  " : ", ");
    }
    sink(arg.long_name);
    if constexpr (Arg::takes_value) {
      sink(" ");
      sink(arg.value_name);
    }
  } else {
    sink(arg.value_name);
    if constexpr (Arg::is_collecting) {
      sink("...");
    }
  }
}

/** Hands the fragments of the argument’s entry in the usage line to `sink`, in order. */
template<typename Arg>
void usage_parts(const Arg& arg, const auto& sink) {
  // A list that has to collect a value is no more optional than a required argument is.
  const bool bracketed = Arg::presence != Presence::required && arg.least_num == 0;
  if (bracketed) {
    sink("[");
  }
  if constexpr (Arg::is_named) {
    sink(arg.short_name.empty() ? arg.long_name : arg.short_name);
    if constexpr (Arg::takes_value) {
      sink(" ");
      sink(arg.value_name);
    }
  } else {
    sink(arg.value_name);
    if constexpr (Arg::is_collecting) {
      sink("...");
    }
  }
  if (bracketed) {
    sink("]");
  }
}

/** Hands the “(choices: …)” and “(default: …)” tails of the argument’s description to `sink`. */
template<typename Arg>
void tail_parts(const Arg& arg, const auto& sink) {
  using Value = Arg::Value;
  if constexpr (Arg::choice_num > 0 && Printable<Value>) {
    write_choices(sink, arg.choice_values);
  } else if constexpr (Arg::takes_value && HasValueNames<Value>) {
    // A type admitting only a fixed set of values lists them even where none were singled out.
    write_choices(sink, argument_value_names(thes::type_tag<Value>));
  }
  if constexpr (Arg::kind == ArgumentKind::list) {
    // The trailing “...” already says “several”, so only a narrower count is worth spelling out.
    if (arg.least_num == arg.most_num) {
      sink(" (count: ");
      write_value(sink, arg.least_num);
      sink(")");
    } else if (arg.most_num != unbounded) {
      sink(" (count: ");
      write_value(sink, arg.least_num);
      sink(" to ");
      write_value(sink, arg.most_num);
      sink(")");
    } else if (arg.least_num > 0) {
      sink(" (count: ");
      write_value(sink, arg.least_num);
      sink(" or more)");
    }
  }
  // Only named arguments are marked: a positional argument is required unless it says otherwise,
  // so saying so on every one of them would be noise, whereas a named argument that has to be
  // given is the exception worth pointing out.
  if constexpr (Arg::is_named && Arg::presence == Presence::required) {
    sink(" (required)");
  }
  // The default of a flag or a counter is implied by what it is, so showing it is only noise.
  if constexpr (Arg::presence == Presence::defaulted && Arg::takes_value && Printable<Value>) {
    sink(" (default: ");
    write_value(sink, arg.default_value);
    sink(")");
  }
}

/** The width of the argument’s entry in the argument list. */
template<typename Arg>
std::size_t label_width(const Arg& arg) {
  std::size_t width = 0;
  label_parts(arg, TextCounter{width});
  return width;
}

/** Writes one row of the argument list: the label, padded to `width`, and the description. */
template<typename Arg>
void write_row(std::FILE* out, const Arg& arg, std::size_t width) {
  write_spaces(out, 2);
  label_parts(arg, TextWriter{out});

  std::size_t description_width = arg.help_text.size();
  tail_parts(arg, TextCounter{description_width});
  if (description_width > 0) {
    write_spaces(out, width - label_width(arg) + 2);
    write_text(out, arg.help_text);
    tail_parts(arg, TextWriter{out});
  }
  write_text(out, "\n");
}

/** Writes one row of the argument list for an argument the parser provides by itself. */
inline void write_plain_row(std::FILE* out, std::string_view label, std::size_t width,
                            std::string_view description) {
  write_spaces(out, 2);
  write_text(out, label);
  write_spaces(out, width - label.size() + 2);
  write_text(out, description);
  write_text(out, "\n");
}
} // namespace detail
} // namespace thes::argparse

#endif // INCLUDE_THESAUROS_ARGPARSE_HELP_HPP
