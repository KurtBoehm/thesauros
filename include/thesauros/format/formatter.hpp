#ifndef INCLUDE_THESAUROS_FORMAT_FORMATTER_HPP
#define INCLUDE_THESAUROS_FORMAT_FORMATTER_HPP

#include "thesauros/format/fmtlib.hpp"

namespace thes {
template<typename T, typename TChar = char>
struct NestedFormatter : ::fmt::nested_formatter<T, TChar> {};

template<typename TChar = char>
struct SimpleFormatter {
  constexpr SimpleFormatter() = default;

  constexpr const TChar* parse(::fmt::parse_context<TChar>& ctx) {
    auto it = ctx.begin();
    auto end = ctx.end();
    if (it == end) {
      return it;
    }
    ::fmt::format_specs specs{};
    it = ::fmt::detail::parse_align(it, end, specs);
    specs_ = specs;
    TChar c = *it;
    auto width_ref = ::fmt::detail::arg_ref<TChar>();
    if ((c >= '0' && c <= '9') || c == '{') {
      it = ::fmt::detail::parse_width(it, end, specs, width_ref, ctx);
      width_ = specs.width;
    }
    ctx.advance_to(it);
    return it;
  }

  template<typename TFormatContext, typename TF>
  auto write_padded(TFormatContext& ctx, TF write) const -> decltype(ctx.out()) {
    if (width_ == 0) {
      return write(ctx.out());
    }
    auto buf = ::fmt::basic_memory_buffer<TChar>();
    write(::fmt::basic_appender<TChar>(buf));
    auto specs = ::fmt::format_specs();
    specs.width = width_;
    specs.set_fill(::fmt::basic_string_view<TChar>(specs_.fill<TChar>(), specs_.fill_size()));
    specs.set_align(specs_.align());
    return ::fmt::detail::write<TChar>(
      ctx.out(), ::fmt::basic_string_view<TChar>(buf.data(), buf.size()), specs);
  }

private:
  ::fmt::basic_specs specs_{};
  int width_{};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_FORMAT_FORMATTER_HPP
