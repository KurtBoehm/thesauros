// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_FORMAT_MACROPOLIS_HPP
#define INCLUDE_THESAUROS_FORMAT_MACROPOLIS_HPP

#include "thesauros/format/debug.hpp"
#include "thesauros/format/fmtlib.hpp"
#include "thesauros/format/formatter.hpp"
#include "thesauros/io/delimiter.hpp"
#include "thesauros/macropolis/type.hpp"

template<thes::HasTypeInfo T>
struct fmt::formatter<T> : public thes::SimpleFormatter<> {
  using Info = thes::TypeInfo<T>;

  auto format(const T& obj, format_context& ctx) const {
    return this->write_padded(ctx, [&](auto it) {
      it = fmt::format_to(it, "{}{{", Info::name);
      thes::Delimiter delim{", "};
      Info::members | thes::star::for_each([&](auto mem) {
        it = fmt::format_to(it, "{}.{}={}", delim, mem.name, thes::debug_view(mem.value(obj)));
      });
      it = fmt::format_to(it, "}}");
      return it;
    });
  }
};

#endif // INCLUDE_THESAUROS_FORMAT_MACROPOLIS_HPP
