#ifndef INCLUDE_THESAUROS_FORMAT_STRING_ESCAPE_HPP
#define INCLUDE_THESAUROS_FORMAT_STRING_ESCAPE_HPP

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/format/simple-formatter.hpp"
#include "thesauros/utility/string-escape.hpp"

template<typename T>
struct fmt::formatter<thes::EscapedPrinter<T>> : public thes::SimpleFormatter {
  auto format(thes::EscapedPrinter<T> p, format_context& ctx) const {
    return this->write_padded(ctx, [&](auto it) { return thes::escape_string(p.value(), it); });
  }
};

#endif // INCLUDE_THESAUROS_FORMAT_STRING_ESCAPE_HPP
