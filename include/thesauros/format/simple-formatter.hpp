#ifndef INCLUDE_THESAUROS_FORMAT_SIMPLE_FORMATTER_HPP
#define INCLUDE_THESAUROS_FORMAT_SIMPLE_FORMATTER_HPP

#include <iterator>

#include "thesauros/format/fmtlib.hpp"

namespace thes {
struct SimpleFormatter {
  constexpr SimpleFormatter() = default;

  FMT_CONSTEXPR const char* parse(fmt::format_parse_context& ctx) {
    auto specs = fmt::detail::dynamic_format_specs<char>();
    const auto* it =
      parse_format_specs(ctx.begin(), ctx.end(), specs, ctx, fmt::detail::type::none_type);
    width_ = specs.width;
    fill_ = specs.fill;
    align_ = specs.align;
    ctx.advance_to(it);
    return it;
  }

  template<typename TOp>
  auto write_padded(fmt::format_context& ctx, TOp write) const -> decltype(ctx.out()) {
    if (width_ == 0) {
      return write(ctx.out());
    }
    auto buf = fmt::memory_buffer();
    write(std::back_inserter(buf));
    auto specs = fmt::format_specs<>();
    specs.width = width_;
    specs.fill = fill_;
    specs.align = align_;
    return fmt::detail::write(ctx.out(), fmt::string_view(buf.data(), buf.size()), specs);
  }

private:
  int width_{0};
  fmt::detail::fill_t<char> fill_;
  fmt::align_t align_ : 4 {fmt::align_t::none};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_FORMAT_SIMPLE_FORMATTER_HPP
