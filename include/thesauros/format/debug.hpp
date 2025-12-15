// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_FORMAT_DEBUG_HPP
#define INCLUDE_THESAUROS_FORMAT_DEBUG_HPP

#include <type_traits>

#include "thesauros/format/fmtlib.hpp"

namespace thes {
template<typename T>
struct DebugPrinter {
  T value;
};

template<typename T>
constexpr DebugPrinter<T> debug_view(T&& value) {
  return DebugPrinter<T>{std::forward<T>(value)};
}
} // namespace thes

template<typename T>
struct fmt::formatter<thes::DebugPrinter<T>> {
  constexpr formatter() {
    detail::maybe_set_debug_format(formatter_, true);
  }

  constexpr const char* parse(fmt::format_parse_context& ctx) {
    return formatter_.parse(ctx);
  }

  auto format(const thes::DebugPrinter<T>& m, format_context& ctx) const {
    return formatter_.format(m.value, ctx);
  }

private:
  fmt::formatter<std::decay_t<T>> formatter_{};
};

#endif // INCLUDE_THESAUROS_FORMAT_DEBUG_HPP
