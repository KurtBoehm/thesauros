#ifndef INCLUDE_THESAUROS_FORMAT_FORMATTER_HPP
#define INCLUDE_THESAUROS_FORMAT_FORMATTER_HPP

#include "thesauros/format/fmtlib.hpp"

namespace thes {
template<typename T, typename TChar = char>
struct NestedFormatter {
  constexpr NestedFormatter() = default;

  constexpr auto parse(::fmt::basic_format_parse_context<TChar>& ctx) -> decltype(ctx.begin()) {
    auto specs = ::fmt::detail::dynamic_format_specs<TChar>();
    auto it =
      parse_format_specs(ctx.begin(), ctx.end(), specs, ctx, ::fmt::detail::type::none_type);
    width_ = specs.width;
    fill_ = specs.fill;
    align_ = specs.align;
    ctx.advance_to(it);
    return formatter_.parse(ctx);
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
    specs.fill = fill_;
    specs.align = align_;
    return ::fmt::detail::write<TChar>(ctx.out(), basic_string_view<TChar>(buf.data(), buf.size()),
                                       specs);
  }

  ::fmt::nested_view<T, TChar> nested(const T& value) const {
    return ::fmt::nested_view<T, TChar>{&formatter_, &value};
  }

protected:
  constexpr const ::fmt::formatter<T>& underlying() const {
    return formatter_;
  }
  constexpr ::fmt::formatter<T>& underlying() {
    return formatter_;
  }

private:
  int width_{};
  ::fmt::detail::fill_t fill_{};
  ::fmt::align_t align_ : 4 {::fmt::align_t::none};
  ::fmt::formatter<T> formatter_{};
};

template<typename TChar = char>
struct SimpleFormatter {
  constexpr SimpleFormatter() = default;

  constexpr auto parse(::fmt::basic_format_parse_context<TChar>& ctx) -> decltype(ctx.begin()) {
    auto specs = ::fmt::detail::dynamic_format_specs<TChar>();
    auto it =
      parse_format_specs(ctx.begin(), ctx.end(), specs, ctx, ::fmt::detail::type::none_type);
    width_ = specs.width;
    fill_ = specs.fill;
    align_ = specs.align;
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
    specs.fill = fill_;
    specs.align = align_;
    return ::fmt::detail::write<TChar>(ctx.out(), basic_string_view<TChar>(buf.data(), buf.size()),
                                       specs);
  }

private:
  int width_{0};
  ::fmt::detail::fill_t fill_;
  ::fmt::align_t align_ : 4 {::fmt::align_t::none};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_FORMAT_FORMATTER_HPP
