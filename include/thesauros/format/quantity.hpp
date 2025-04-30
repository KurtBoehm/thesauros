// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_FORMAT_QUANTITY_HPP
#define INCLUDE_THESAUROS_FORMAT_QUANTITY_HPP

#include <cstdint>

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/format/formatter.hpp"
#include "thesauros/quantity.hpp"

template<typename TRep, typename TUnit>
struct fmt::formatter<thes::Quantity<TRep, TUnit>> : fmt::nested_formatter<TRep> {
  auto format(thes::Quantity<TRep, TUnit> q, format_context& ctx) const {
    return this->write_padded(ctx, [&](auto out) {
      return fmt::format_to(out, "{} {}", this->nested(q.count()), thes::unit_name<TUnit>);
    });
  }
};

template<thes::AnyQuantity TQuantity>
struct fmt::formatter<thes::SplitTimePrinter<TQuantity>> : thes::SimpleFormatter<> {
  using Self = thes::SplitTimePrinter<TQuantity>;

  auto format(Self p, format_context& ctx) const {
    return this->write_padded(ctx, [&](auto it) {
      std::intmax_t milliq =
        quantity_cast<thes::Quantity<std::intmax_t, typename Self::Unit>>(p.quantity).count();

      if (milliq < 0) {
        it = fmt::format_to(it, "-");
        milliq = -milliq;
      }

      const auto hour_div = milliq / Self::hour_limit;
      const auto hour_mod = milliq % Self::hour_limit;
      const auto min_div = hour_mod / Self::min_limit;
      const auto min_mod = hour_mod % Self::min_limit;
      const auto sec_div = min_mod / Self::sec_limit;
      const auto sec_mod = min_mod % Self::sec_limit;
      it = fmt::format_to(it, "{}:{:02}:{:02}", hour_div, min_div, sec_div);

      if (milliq < Self::min_limit) {
        it = fmt::format_to(it, ".{:03}", sec_mod);
      }

      return it;
    });
  }
};

#endif // INCLUDE_THESAUROS_FORMAT_QUANTITY_HPP
