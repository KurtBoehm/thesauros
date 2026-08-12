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
#include <ranges>
#include <span>
#include <string_view>

#include "thesauros/argparse/argument.hpp"
#include "thesauros/argparse/style.hpp"
#include "thesauros/format/fmtlib.hpp"
#include "thesauros/types/type-tag.hpp"

namespace thes::argparse::detail {
/**
 * A sink writing the fragments it is handed to `out`.
 *
 * The help text is described once, as a sequence of fragments handed to a sink, and is then either
 * written or measured depending on which sink receives it. That is what keeps the column widths
 * measured over the text alone while the written form carries the styling: `TextCounter` adds up
 * what it is handed and disregards how it is styled.
 */
struct TextWriter {
  std::FILE* out;

  void operator()(std::string_view text) const {
    fmt::print(out, "{}", text);
  }
  void operator()(const fmt::text_style& style, std::string_view text) const {
    fmt::print(out, style, "{}", text);
  }
  /** Writes one styled run, so that a whole tail costs a single pair of escape sequences. */
  template<typename... Ts>
  void format(const fmt::text_style& style, fmt::format_string<Ts...> spec, Ts&&... args) const {
    fmt::print(out, style, spec, std::forward<Ts>(args)...);
  }
};

/** A sink adding up the widths of the fragments it is handed. */
struct TextCounter {
  std::size_t& total;

  void operator()(std::string_view text) const {
    total += text.size();
  }
  void operator()(const fmt::text_style& /*style*/, std::string_view text) const {
    total += text.size();
  }
  template<typename... Ts>
  void format(const fmt::text_style& /*style*/, fmt::format_string<Ts...> spec,
              Ts&&... args) const {
    total += fmt::formatted_size(spec, std::forward<Ts>(args)...);
  }
};

/**
 * Whether `argument_value_text` is defined for `T` and findable by argument-dependent lookup, which
 * is how a type {fmt} cannot format on its own says how its values are written.
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
concept Printable = HasValueText<T> || fmt::formattable<T>;

/** `value` in the form it would take on the command line, as something {fmt} can format. */
template<Printable T>
auto value_text(const T& value) {
  if constexpr (HasValueText<T>) {
    return std::string_view{argument_value_text(thes::type_tag<T>, value)};
  } else {
    return value;
  }
}

/** Hands a “(choices: …)” tail listing `values` to `sink`. */
void write_choices(const auto& sink, const fmt::text_style& style, const auto& values) {
  const auto texts = std::views::transform(values, [](const auto& v) { return value_text(v); });
  sink.format(style, " (choices: {})", fmt::join(texts, ", "));
}

/** Hands the fragments of the argument’s entry in the argument list to `sink`, in order. */
template<typename Arg>
void label_parts(const Arg& arg, const auto& sink, const HelpStyle& style) {
  if constexpr (Arg::is_named) {
    if (arg.short_name.empty()) {
      sink("    ");
    } else {
      sink(style.name, arg.short_name);
      sink(arg.long_name.empty() ? "  " : ", ");
    }
    sink(style.name, arg.long_name);
    if constexpr (Arg::takes_value) {
      sink(" ");
      sink(style.value, arg.value_name);
    }
  } else {
    sink(style.value, arg.value_name);
    if constexpr (Arg::is_collecting) {
      sink("...");
    }
  }
}

/** Hands the fragments of the argument’s entry in the usage line to `sink`, in order. */
template<typename Arg>
void usage_parts(const Arg& arg, const auto& sink, const HelpStyle& style) {
  // A list that has to collect a value is no more optional than a required argument is.
  const bool bracketed = Arg::presence != Presence::required && arg.least_num == 0;
  if (bracketed) {
    sink("[");
  }
  if constexpr (Arg::is_named) {
    sink(style.name, arg.short_name.empty() ? arg.long_name : arg.short_name);
    if constexpr (Arg::takes_value) {
      sink(" ");
      sink(style.value, arg.value_name);
    }
  } else {
    sink(style.value, arg.value_name);
    if constexpr (Arg::is_collecting) {
      sink("...");
    }
  }
  if (bracketed) {
    sink("]");
  }
}

/**
 * Hands the “(choices: …)”, “(count: …)”, “(required)” and “(default: …)” tails of the argument’s
 * description to `sink`, in order.
 */
template<typename Arg>
void tail_parts(const Arg& arg, const auto& sink, const HelpStyle& style) {
  using Value = Arg::Value;
  const fmt::text_style& tail = style.tail;

  if constexpr (Arg::choice_num > 0 && Printable<Value>) {
    write_choices(sink, tail, arg.choice_values);
  } else if constexpr (Arg::takes_value && HasValueNames<Value>) {
    // A type admitting only a fixed set of values lists them even where none were singled out.
    write_choices(sink, tail, argument_value_names(thes::type_tag<Value>));
  }

  if constexpr (Arg::kind == ArgumentKind::list) {
    // The trailing “...” already says “several”, so only a narrower count is worth spelling out.
    if (arg.least_num == arg.most_num) {
      sink.format(tail, " (count: {})", arg.least_num);
    } else if (arg.most_num != unbounded) {
      sink.format(tail, " (count: {} to {})", arg.least_num, arg.most_num);
    } else if (arg.least_num > 0) {
      sink.format(tail, " (count: {} or more)", arg.least_num);
    }
  }

  // Only named arguments are marked: a positional argument is required unless it says otherwise,
  // so saying so on every one of them would be noise, whereas a named argument that has to be
  // given is the exception worth pointing out.
  if constexpr (Arg::is_named && Arg::presence == Presence::required) {
    sink(tail, " (required)");
  }
  // The default of a flag or a counter is implied by what it is, so showing it is only noise.
  if constexpr (Arg::presence == Presence::defaulted && Arg::takes_value && Printable<Value>) {
    sink.format(tail, " (default: {})", value_text(arg.default_value));
  }
}

/** The width of the argument’s entry in the argument list. */
template<typename Arg>
std::size_t label_width(const Arg& arg, const HelpStyle& style) {
  std::size_t width = 0;
  label_parts(arg, TextCounter{width}, style);
  return width;
}

/** Writes one row of the argument list: the label, padded to `width`, and the description. */
template<typename Arg>
void write_row(std::FILE* out, const Arg& arg, std::size_t width, const HelpStyle& style) {
  const TextWriter sink{out};
  fmt::print(out, "  ");
  label_parts(arg, sink, style);

  std::size_t description_width = arg.help_text.size();
  tail_parts(arg, TextCounter{description_width}, style);
  if (description_width > 0) {
    fmt::print(out, "{:{}}{}", "", width - label_width(arg, style) + 2, arg.help_text);
    tail_parts(arg, sink, style);
  }
  fmt::print(out, "\n");
}

/** Writes one row of the argument list for an argument the parser provides by itself. */
inline void write_plain_row(std::FILE* out, std::string_view label, std::size_t width,
                            std::string_view description, const HelpStyle& style) {
  fmt::print(out, "  ");
  fmt::print(out, style.name, "{}", label);
  fmt::print(out, "{:{}}{}\n", "", width - label.size() + 2, description);
}
} // namespace thes::argparse::detail

#endif // INCLUDE_THESAUROS_ARGPARSE_HELP_HPP
