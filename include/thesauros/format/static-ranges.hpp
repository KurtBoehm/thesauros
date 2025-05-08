// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_FORMAT_STATIC_RANGES_HPP
#define INCLUDE_THESAUROS_FORMAT_STATIC_RANGES_HPP

#include <functional>

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/format/formatter.hpp"
#include "thesauros/static-ranges/definitions.hpp"
#include "thesauros/static-ranges/sinks/for-each.hpp"
#include "thesauros/static-ranges/views/iota.hpp"

namespace thes::detail {
template<thes::star::AnyStaticRange TRange>
inline auto static_range_format(const TRange& range, auto it, auto op) {
  it = fmt::format_to(it, "[");
  star::iota<0, star::size<TRange>> | star::for_each([&](auto i) {
    if constexpr (i > 0) {
      it = fmt::format_to(it, ", ");
    }
    it = fmt::format_to(it, "{}", op(star::get_at(range, i)));
  });
  it = fmt::format_to(it, "]");
  return it;
}
} // namespace thes::detail

template<thes::star::AnyStaticRange TRange>
requires(thes::star::HasValue<TRange> && thes::star::AnyPrintable<TRange>)
struct fmt::formatter<TRange> : public fmt::nested_formatter<thes::star::Value<TRange>> {
  auto format(const TRange& r, fmt::format_context& ctx) const {
    return this->write_padded(ctx, [&](auto it) {
      return thes::detail::static_range_format(r, it,
                                               [&](const auto& v) { return this->nested(v); });
    });
  }
};
template<thes::star::AnyStaticRange TRange>
requires(!thes::star::HasValue<TRange> && thes::star::AnyPrintable<TRange>)
struct fmt::formatter<TRange> : public thes::SimpleFormatter<> {
  auto format(const TRange& r, fmt::format_context& ctx) const {
    return this->write_padded(
      ctx, [&](auto it) { return thes::detail::static_range_format(r, it, std::identity{}); });
  }
};

#endif // INCLUDE_THESAUROS_FORMAT_STATIC_RANGES_HPP
