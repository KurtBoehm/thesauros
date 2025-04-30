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
#include "thesauros/static-ranges/ranges/iota.hpp"
#include "thesauros/static-ranges/sinks/for-each.hpp"
#include "thesauros/static-ranges/sinks/format.hpp"

namespace thes::detail {
template<thes::star::AnyStaticRange TRange>
inline auto static_range_format(const star::Formatter<TRange>& f, auto it, auto op) {
  it = fmt::format_to(it, "[");
  star::iota<0, star::size<TRange>> | star::for_each([&](auto i) {
    if constexpr (i > 0) {
      it = fmt::format_to(it, ", ");
    }
    it = fmt::format_to(it, "{}", op(star::get_at(f.range(), i)));
  });
  it = fmt::format_to(it, "]");
  return it;
}
} // namespace thes::detail

template<thes::star::AnyStaticRange TRange>
requires(thes::star::HasValue<TRange>)
struct fmt::formatter<thes::star::Formatter<TRange>>
    : public fmt::nested_formatter<thes::star::Value<TRange>> {
  auto format(const thes::star::Formatter<TRange>& f, fmt::format_context& ctx) const {
    return this->write_padded(ctx, [&](auto it) {
      return thes::detail::static_range_format(f, it,
                                               [&](const auto& v) { return this->nested(v); });
    });
  }
};
template<thes::star::AnyStaticRange TRange>
requires(!thes::star::HasValue<TRange>)
struct fmt::formatter<thes::star::Formatter<TRange>> : public thes::SimpleFormatter<> {
  auto format(const thes::star::Formatter<TRange>& f, fmt::format_context& ctx) const {
    return this->write_padded(
      ctx, [&](auto it) { return thes::detail::static_range_format(f, it, std::identity{}); });
  }
};

#endif // INCLUDE_THESAUROS_FORMAT_STATIC_RANGES_HPP
