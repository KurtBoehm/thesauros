#ifndef INCLUDE_THESAUROS_FORMAT_JSON_HPP
#define INCLUDE_THESAUROS_FORMAT_JSON_HPP

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/format/simple-formatter.hpp"
#include "thesauros/io/json.hpp"

template<typename T>
struct fmt::formatter<thes::JsonPrinter<T>> : public thes::SimpleFormatter {
  auto format(thes::JsonPrinter<T> p, format_context& ctx) const {
    return this->write_padded(ctx, [&](auto it) { return write_json(it, p.value(), p.indent()); });
  }
};

#endif // INCLUDE_THESAUROS_FORMAT_JSON_HPP
