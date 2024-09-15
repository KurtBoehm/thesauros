#ifndef INCLUDE_THESAUROS_FORMAT_DELIMITER_HPP
#define INCLUDE_THESAUROS_FORMAT_DELIMITER_HPP

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/format/formatter.hpp"
#include "thesauros/io/delimiter.hpp"

template<>
struct fmt::formatter<thes::Delimiter> : public thes::SimpleFormatter<> {
  auto format(const thes::Delimiter& delim, format_context& ctx) const {
    return this->write_padded(ctx, [&](auto it) { return delim.output(it); });
  }
};

#endif // INCLUDE_THESAUROS_FORMAT_DELIMITER_HPP
