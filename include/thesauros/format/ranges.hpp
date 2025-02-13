#ifndef INCLUDE_THESAUROS_FORMAT_RANGES_HPP
#define INCLUDE_THESAUROS_FORMAT_RANGES_HPP

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/ranges/iota.hpp"

template<typename T>
struct fmt::formatter<thes::IotaRange<T>> : fmt::nested_formatter<T> {
  auto format(const thes::IotaRange<T> range, format_context& ctx) const {
    return this->write_padded(ctx, [&](auto out) {
      return fmt::format_to(out, "[{}, {})", this->nested(range.begin_value()),
                            this->nested(range.end_value()));
    });
  }
};
template<typename T>
struct fmt::range_format_kind<thes::IotaRange<T>, char> {
  static constexpr auto value = fmt::range_format::disabled;
};

#endif // INCLUDE_THESAUROS_FORMAT_RANGES_HPP
