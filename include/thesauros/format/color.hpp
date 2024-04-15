#ifndef INCLUDE_THESAUROS_FORMAT_COLOR_HPP
#define INCLUDE_THESAUROS_FORMAT_COLOR_HPP

#include <array>
#include <cstddef>
#include <type_traits>

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/utility/value-tag.hpp"

namespace thes {
inline constexpr fmt::text_style fg_black{fmt::fg(fmt::terminal_color::black)};
inline constexpr fmt::text_style fg_red{fmt::fg(fmt::terminal_color::red)};
inline constexpr fmt::text_style fg_green{fmt::fg(fmt::terminal_color::green)};
inline constexpr fmt::text_style fg_yellow{fmt::fg(fmt::terminal_color::yellow)};
inline constexpr fmt::text_style fg_blue{fmt::fg(fmt::terminal_color::blue)};
inline constexpr fmt::text_style fg_magenta{fmt::fg(fmt::terminal_color::magenta)};
inline constexpr fmt::text_style fg_cyan{fmt::fg(fmt::terminal_color::cyan)};
inline constexpr fmt::text_style fg_white{fmt::fg(fmt::terminal_color::white)};
inline constexpr fmt::text_style fg_bright_black{fmt::fg(fmt::terminal_color::bright_black)};
inline constexpr fmt::text_style fg_bright_red{fmt::fg(fmt::terminal_color::bright_red)};
inline constexpr fmt::text_style fg_bright_green{fmt::fg(fmt::terminal_color::bright_green)};
inline constexpr fmt::text_style fg_bright_yellow{fmt::fg(fmt::terminal_color::bright_yellow)};
inline constexpr fmt::text_style fg_bright_blue{fmt::fg(fmt::terminal_color::bright_blue)};
inline constexpr fmt::text_style fg_bright_magenta{fmt::fg(fmt::terminal_color::bright_magenta)};
inline constexpr fmt::text_style fg_bright_cyan{fmt::fg(fmt::terminal_color::bright_cyan)};
inline constexpr fmt::text_style fg_bright_white{fmt::fg(fmt::terminal_color::bright_white)};

inline constexpr std::array fg_rainbow{
  fg_red, fg_yellow, fg_green, fg_cyan, fg_blue, fg_magenta,
};
inline constexpr std::array fg_bright_rainbow{
  fg_bright_red,  fg_bright_yellow, fg_bright_green,
  fg_bright_cyan, fg_bright_blue,   fg_bright_magenta,
};

inline constexpr fmt::text_style rainbow_fg(std::size_t idx) {
  return fg_rainbow[idx % fg_rainbow.size()];
}
inline fmt::text_style next_rainbow_fg() {
  static std::size_t idx = 0;
  return rainbow_fg(idx++);
}

inline constexpr fmt::text_style bright_rainbow_fg(std::size_t idx) {
  return fg_bright_rainbow[idx % fg_bright_rainbow.size()];
}
inline fmt::text_style next_bright_rainbow_fg() {
  static std::size_t idx = 0;
  return bright_rainbow_fg(idx++);
}

inline constexpr fmt::text_style bg_black{fmt::bg(fmt::terminal_color::black)};
inline constexpr fmt::text_style bg_red{fmt::bg(fmt::terminal_color::red)};
inline constexpr fmt::text_style bg_green{fmt::bg(fmt::terminal_color::green)};
inline constexpr fmt::text_style bg_yellow{fmt::bg(fmt::terminal_color::yellow)};
inline constexpr fmt::text_style bg_blue{fmt::bg(fmt::terminal_color::blue)};
inline constexpr fmt::text_style bg_magenta{fmt::bg(fmt::terminal_color::magenta)};
inline constexpr fmt::text_style bg_cyan{fmt::bg(fmt::terminal_color::cyan)};
inline constexpr fmt::text_style bg_white{fmt::bg(fmt::terminal_color::white)};
inline constexpr fmt::text_style bg_bright_black{fmt::bg(fmt::terminal_color::bright_black)};
inline constexpr fmt::text_style bg_bright_red{fmt::bg(fmt::terminal_color::bright_red)};
inline constexpr fmt::text_style bg_bright_green{fmt::bg(fmt::terminal_color::bright_green)};
inline constexpr fmt::text_style bg_bright_yellow{fmt::bg(fmt::terminal_color::bright_yellow)};
inline constexpr fmt::text_style bg_bright_blue{fmt::bg(fmt::terminal_color::bright_blue)};
inline constexpr fmt::text_style bg_bright_magenta{fmt::bg(fmt::terminal_color::bright_magenta)};
inline constexpr fmt::text_style bg_bright_cyan{fmt::bg(fmt::terminal_color::bright_cyan)};
inline constexpr fmt::text_style bg_bright_white{fmt::bg(fmt::terminal_color::bright_white)};

inline constexpr fmt::text_style bold{fmt::emphasis::bold};
inline constexpr fmt::text_style faint{fmt::emphasis::faint};
inline constexpr fmt::text_style italic{fmt::emphasis::italic};
inline constexpr fmt::text_style underline{fmt::emphasis::underline};
inline constexpr fmt::text_style blink{fmt::emphasis::blink};
inline constexpr fmt::text_style reverse{fmt::emphasis::reverse};
inline constexpr fmt::text_style conceal{fmt::emphasis::conceal};
inline constexpr fmt::text_style strikethrough{fmt::emphasis::strikethrough};

template<bool tFormat>
struct FormattingTag : public BoolTag<tFormat> {};
inline constexpr FormattingTag<true> format_tag{};
inline constexpr FormattingTag<false> unformat_tag{};
template<typename T>
struct IsFormatTagTrait : public std::false_type {};
template<bool tIsFormat>
struct IsFormatTagTrait<FormattingTag<tIsFormat>> : public std::true_type {};
template<typename T>
concept AnyFormatTag = IsFormatTagTrait<T>::value;

template<typename TFmt, typename... TArgs>
inline void tsprint(FormattingTag<true> /*tag*/, const ::fmt::text_style& ts, const TFmt& fmt,
                    const TArgs&... args) {
  ::fmt::print(ts, fmt, args...);
}
template<typename TFmt, typename... TArgs>
inline void tsprint(FormattingTag<false> /*tag*/, const ::fmt::text_style& /*ts*/,
                    ::fmt::format_string<TArgs...> fmt, TArgs&&... args) {
  ::fmt::print(fmt, std::forward<TArgs>(args)...);
}

template<typename TFmt, typename... TArgs>
inline auto tsformat_to(FormattingTag<true> /*tag*/, auto it, const ::fmt::text_style& ts,
                        const TFmt& fmt, const TArgs&... args) {
  return ::fmt::format_to(it, ts, fmt, args...);
}
template<typename TFmt, typename... TArgs>
inline auto tsformat_to(FormattingTag<false> /*tag*/, auto it, const ::fmt::text_style& /*ts*/,
                        ::fmt::format_string<TArgs...> fmt, TArgs&&... args) {
  return ::fmt::format_to(it, fmt, std::forward<TArgs>(args)...);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_FORMAT_COLOR_HPP
