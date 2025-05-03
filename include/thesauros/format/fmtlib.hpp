// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_FORMAT_FMTLIB_HPP
#define INCLUDE_THESAUROS_FORMAT_FMTLIB_HPP

#include "thesauros/macropolis/warnings.hpp"

THES_POLIS_DIAGNOSTICS_IGNORED_PUSH(gcc, "-Wsign-conversion")
// IWYU pragma: begin_exports
#include <fmt/base.h>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
// IWYU pragma: end_exports
THES_POLIS_DIAGNOSTICS_IGNORED_POP(gcc)

#endif // INCLUDE_THESAUROS_FORMAT_FMTLIB_HPP
