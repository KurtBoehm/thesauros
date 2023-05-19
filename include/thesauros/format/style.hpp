#ifndef INCLUDE_THESAUROS_FORMAT_STYLE_HPP
#define INCLUDE_THESAUROS_FORMAT_STYLE_HPP

#include <concepts>
#include <cstdint>
#include <ostream>
#include <unordered_map>

namespace thes::fmt {
enum struct Colour : std::uint8_t {
  NONE,
  BLACK,
  RED,
  GREEN,
  YELLOW,
  BLUE,
  MAGENTA,
  CYAN,
  WHITE,
  BRIGHT_BLACK,
  BRIGHT_RED,
  BRIGHT_GREEN,
  BRIGHT_YELLOW,
  BRIGHT_BLUE,
  BRIGHT_MAGENTA,
  BRIGHT_CYAN,
  BRIGHT_WHITE
};

enum struct Intensity : std::uint8_t {
  NORMAL,
  BOLD,
  FAINT,
};

struct Style {
  Colour fg = Colour::NONE;
  Colour bg = Colour::NONE;
  Intensity intensity = Intensity::NORMAL;
  bool italic : 1 = false;
  bool underline : 1 = false;
  bool blinking : 1 = false;
  bool reversed : 1 = false;
  bool concealed : 1 = false;
  bool crossed_out : 1 = false;

  void apply_fg(std::ostream& s) const {
    switch (fg) {
    case Colour::NONE: s << "\033[39m"; break;
    case Colour::BLACK: s << "\033[30m"; break;
    case Colour::RED: s << "\033[31m"; break;
    case Colour::GREEN: s << "\033[32m"; break;
    case Colour::YELLOW: s << "\033[33m"; break;
    case Colour::BLUE: s << "\033[34m"; break;
    case Colour::MAGENTA: s << "\033[35m"; break;
    case Colour::CYAN: s << "\033[36m"; break;
    case Colour::WHITE: s << "\033[37m"; break;
    case Colour::BRIGHT_BLACK: s << "\033[90m"; break;
    case Colour::BRIGHT_RED: s << "\033[91m"; break;
    case Colour::BRIGHT_GREEN: s << "\033[92m"; break;
    case Colour::BRIGHT_YELLOW: s << "\033[93m"; break;
    case Colour::BRIGHT_BLUE: s << "\033[94m"; break;
    case Colour::BRIGHT_MAGENTA: s << "\033[95m"; break;
    case Colour::BRIGHT_CYAN: s << "\033[96m"; break;
    case Colour::BRIGHT_WHITE: s << "\033[97m"; break;
    default: break;
    }
  }
  void apply_bg(std::ostream& s) const {
    switch (bg) {
    case Colour::NONE: s << "\033[49m"; break;
    case Colour::BLACK: s << "\033[40m"; break;
    case Colour::RED: s << "\033[41m"; break;
    case Colour::GREEN: s << "\033[42m"; break;
    case Colour::YELLOW: s << "\033[43m"; break;
    case Colour::BLUE: s << "\033[44m"; break;
    case Colour::MAGENTA: s << "\033[45m"; break;
    case Colour::CYAN: s << "\033[46m"; break;
    case Colour::WHITE: s << "\033[47m"; break;
    case Colour::BRIGHT_BLACK: s << "\033[100m"; break;
    case Colour::BRIGHT_RED: s << "\033[101m"; break;
    case Colour::BRIGHT_GREEN: s << "\033[102m"; break;
    case Colour::BRIGHT_YELLOW: s << "\033[103m"; break;
    case Colour::BRIGHT_BLUE: s << "\033[104m"; break;
    case Colour::BRIGHT_MAGENTA: s << "\033[105m"; break;
    case Colour::BRIGHT_CYAN: s << "\033[106m"; break;
    case Colour::BRIGHT_WHITE: s << "\033[107m"; break;
    default: break;
    }
  }

  void apply_intensity(std::ostream& s) const {
    switch (intensity) {
    case Intensity::NORMAL: s << "\033[22m"; break;
    case Intensity::BOLD: s << "\033[1m"; break;
    case Intensity::FAINT: s << "\033[2m"; break;
    default: break;
    }
  }
  void apply_italic(std::ostream& s) const {
    s << (italic ? "\033[3m" : "\033[23m");
  }
  void apply_underline(std::ostream& s) const {
    s << (underline ? "\033[4m" : "\033[24m");
  }
  void apply_blinking(std::ostream& s) const {
    s << (blinking ? "\033[5m" : "\033[25m");
  }
  void apply_reversed(std::ostream& s) const {
    s << (reversed ? "\033[7m" : "\033[27m");
  }
  void apply_concealed(std::ostream& s) const {
    s << (reversed ? "\033[8m" : "\033[28m");
  }
  void apply_crossed_out(std::ostream& s) const {
    s << (reversed ? "\033[9m" : "\033[29m");
  }

  void apply_diff(std::ostream& s, const Style& other) const {
    if (fg != other.fg) {
      apply_fg(s);
    }
    if (bg != other.bg) {
      apply_bg(s);
    }
    if (intensity != other.intensity) {
      apply_intensity(s);
    }
    if (italic != other.italic) {
      apply_italic(s);
    }
    if (underline != other.underline) {
      apply_underline(s);
    }
    if (blinking != other.blinking) {
      apply_blinking(s);
    }
    if (reversed != other.reversed) {
      apply_reversed(s);
    }
    if (concealed != other.concealed) {
      apply_concealed(s);
    }
    if (crossed_out != other.crossed_out) {
      apply_crossed_out(s);
    }
  }

  constexpr bool operator==(const Style& s) const = default;
};

struct Foreground {
  Colour colour;

  explicit constexpr Foreground(Colour c) : colour(c) {}

  void apply(Style& s) const {
    s.fg = colour;
  }
};
struct Background {
  Colour colour;

  explicit constexpr Background(Colour c) : colour(c) {}

  void apply(Style& s) const {
    s.bg = colour;
  }
};
struct IntensityApplier {
  Intensity intensity;

  explicit constexpr IntensityApplier(Intensity i) : intensity(i) {}

  void apply(Style& s) const {
    s.intensity = intensity;
  }
};

#define THES_DEFINE_STYLE_APPLIER(STRUCT_NAME, ATTRIBUTE_NAME) \
  template<bool tApplyOrInvert> \
  struct STRUCT_NAME { \
    void apply(Style& s) const { \
      s.ATTRIBUTE_NAME = tApplyOrInvert; \
    } \
  }

THES_DEFINE_STYLE_APPLIER(Italic, italic);
THES_DEFINE_STYLE_APPLIER(Underline, underline);
THES_DEFINE_STYLE_APPLIER(Blinking, blinking);
THES_DEFINE_STYLE_APPLIER(Reversed, reversed);
THES_DEFINE_STYLE_APPLIER(Concealed, concealed);
THES_DEFINE_STYLE_APPLIER(CrossedOut, crossed_out);

#undef THES_DEFINE_STYLE_APPLIER

inline constexpr Foreground fg_none{Colour::NONE};
inline constexpr Foreground fg_black{Colour::BLACK};
inline constexpr Foreground fg_red{Colour::RED};
inline constexpr Foreground fg_green{Colour::GREEN};
inline constexpr Foreground fg_yellow{Colour::YELLOW};
inline constexpr Foreground fg_blue{Colour::BLUE};
inline constexpr Foreground fg_magenta{Colour::MAGENTA};
inline constexpr Foreground fg_cyan{Colour::CYAN};
inline constexpr Foreground fg_white{Colour::WHITE};
inline constexpr Foreground fg_bright_black{Colour::BRIGHT_BLACK};
inline constexpr Foreground fg_bright_red{Colour::BRIGHT_RED};
inline constexpr Foreground fg_bright_green{Colour::BRIGHT_GREEN};
inline constexpr Foreground fg_bright_yellow{Colour::BRIGHT_YELLOW};
inline constexpr Foreground fg_bright_blue{Colour::BRIGHT_BLUE};
inline constexpr Foreground fg_bright_magenta{Colour::BRIGHT_MAGENTA};
inline constexpr Foreground fg_bright_cyan{Colour::BRIGHT_CYAN};
inline constexpr Foreground fg_bright_white{Colour::BRIGHT_WHITE};

inline constexpr Background bg_none{Colour::NONE};
inline constexpr Background bg_black{Colour::BLACK};
inline constexpr Background bg_red{Colour::RED};
inline constexpr Background bg_green{Colour::GREEN};
inline constexpr Background bg_yellow{Colour::YELLOW};
inline constexpr Background bg_blue{Colour::BLUE};
inline constexpr Background bg_magenta{Colour::MAGENTA};
inline constexpr Background bg_cyan{Colour::CYAN};
inline constexpr Background bg_white{Colour::WHITE};
inline constexpr Background bg_bright_black{Colour::BRIGHT_BLACK};
inline constexpr Background bg_bright_red{Colour::BRIGHT_RED};
inline constexpr Background bg_bright_green{Colour::BRIGHT_GREEN};
inline constexpr Background bg_bright_yellow{Colour::BRIGHT_YELLOW};
inline constexpr Background bg_bright_blue{Colour::BRIGHT_BLUE};
inline constexpr Background bg_bright_magenta{Colour::BRIGHT_MAGENTA};
inline constexpr Background bg_bright_cyan{Colour::BRIGHT_CYAN};
inline constexpr Background bg_bright_white{Colour::BRIGHT_WHITE};

inline constexpr IntensityApplier normal_intensity{Intensity::NORMAL};
inline constexpr IntensityApplier bold{Intensity::BOLD};
inline constexpr IntensityApplier faint{Intensity::FAINT};

inline constexpr Italic<true> italic{};
inline constexpr Underline<true> underline{};
inline constexpr Blinking<true> blinking{};
inline constexpr Reversed<true> reversed{};
inline constexpr Concealed<true> concealed{};
inline constexpr CrossedOut<true> crossed_out{};

inline constexpr Italic<false> not_italic{};
inline constexpr Underline<false> not_underline{};
inline constexpr Blinking<false> not_blinking{};
inline constexpr Reversed<false> not_reversed{};
inline constexpr Concealed<false> not_concealed{};
inline constexpr CrossedOut<false> not_crossed_out{};

template<typename TApp>
concept StyleApplier = requires(const TApp& app, Style& s) {
  { app.apply(s) } -> std::same_as<void>;
};

struct StyleContext {
  std::ostream& stream;
  Style previous;

  explicit StyleContext(std::ostream& s) : stream(s), previous(get(s)) {}
  StyleContext(const StyleContext&) = delete;
  StyleContext(StyleContext&&) = delete;
  StyleContext& operator=(const StyleContext&) = delete;
  StyleContext& operator=(StyleContext&&) = delete;
  ~StyleContext() {
    Style& style = get(stream);
    previous.apply_diff(stream, style);

    if (previous != Style{}) {
      style = previous;
    } else {
      remove(stream);
    }
  }

  template<StyleApplier TApp>
  void set(const TApp& app) {
    Style& style = get(stream);
    Style new_style = style;

    app.apply(new_style);
    new_style.apply_diff(stream, style);

    style = new_style;
  }

private:
  using Map = std::unordered_map<std::ostream*, Style>;
  static Map& get_map() {
    static Map map{};
    return map;
  }

  static Style& get(std::ostream& s) {
    return get_map().insert({&s, {}}).first->second;
  }

  static void remove(std::ostream& s) {
    get_map().erase(&s);
  }
};
} // namespace thes::fmt

#endif // INCLUDE_THESAUROS_FORMAT_STYLE_HPP
