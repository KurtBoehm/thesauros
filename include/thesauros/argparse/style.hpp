// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_ARGPARSE_STYLE_HPP
#define INCLUDE_THESAUROS_ARGPARSE_STYLE_HPP

#include "thesauros/format/fmtlib.hpp"

namespace thes::argparse {
/**
 * The text styles the help text and the error messages are written with, one per role rather than
 * one per colour, so that a caller can restyle the output without knowing where each role occurs.
 * The column widths are measured over the text alone, so styling never disturbs the alignment.
 *
 * A default-constructed style writes no escape sequences at all, which is what `plain_style` is.
 */
struct HelpStyle {
  /** The section headings and the “Usage:” label. */
  fmt::text_style heading{};
  /** The short and long names of an argument. */
  fmt::text_style name{};
  /** The placeholder standing for the value of an argument. */
  fmt::text_style value{};
  /** The “(choices: …)”, “(count: …)”, “(default: …)” and “(required)” tails. */
  fmt::text_style tail{};
  /** The word “error” in an error message. */
  fmt::text_style error{};
};

/** A style writing no escape sequences, for output that is redirected or read by a program. */
inline constexpr HelpStyle plain_style{};

/**
 * A restrained style: bold headings, bold cyan names, italic value placeholders, faint tails and a
 * bold red “error”. Italics are the one part of this that a few terminals ignore, in which case the
 * placeholders simply come out unstyled.
 */
inline constexpr HelpStyle ansi_style{
  .heading = fmt::emphasis::bold,
  .name = fmt::emphasis::bold | fmt::fg(fmt::terminal_color::cyan),
  .value = fmt::emphasis::italic,
  .tail = fmt::emphasis::faint,
  .error = fmt::emphasis::bold | fmt::fg(fmt::terminal_color::red),
};
} // namespace thes::argparse

#endif // INCLUDE_THESAUROS_ARGPARSE_STYLE_HPP
