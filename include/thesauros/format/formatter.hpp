// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_FORMAT_FORMATTER_HPP
#define INCLUDE_THESAUROS_FORMAT_FORMATTER_HPP

#include "thesauros/format/fmtlib.hpp"

namespace thes {
template<typename C = char>
struct SimpleFormatter {
  constexpr SimpleFormatter() = default;

  constexpr const C* parse(fmt::parse_context<C>& ctx) {
    auto it = ctx.begin();
    auto end = ctx.end();
    if (it == end) {
      return it;
    }
    fmt::format_specs specs{};
    it = fmt::detail::parse_align(it, end, specs);
    specs_ = specs;
    C c = *it;
    auto width_ref = fmt::detail::arg_ref<C>();
    if ((c >= '0' && c <= '9') || c == '{') {
      it = fmt::detail::parse_width(it, end, specs, width_ref, ctx);
      width_ = specs.width;
    }
    ctx.advance_to(it);
    return it;
  }

  template<typename Ctx, typename F>
  auto write_padded(Ctx& ctx, F write) const -> decltype(ctx.out()) {
    if (width_ == 0) {
      return write(ctx.out());
    }
    auto buf = fmt::basic_memory_buffer<C>();
    write(fmt::basic_appender<C>(buf));
    auto specs = fmt::format_specs();
    specs.width = width_;
    specs.set_fill(fmt::basic_string_view<C>(specs_.fill<C>(), specs_.fill_size()));
    specs.set_align(specs_.align());
    return fmt::detail::write<C>(ctx.out(), fmt::basic_string_view<C>(buf.data(), buf.size()),
                                 specs);
  }

private:
  fmt::basic_specs specs_{};
  int width_{};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_FORMAT_FORMATTER_HPP
