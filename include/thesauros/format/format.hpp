#ifndef INCLUDE_THESAUROS_FORMAT_FORMAT_HPP
#define INCLUDE_THESAUROS_FORMAT_FORMAT_HPP

#include <concepts>
#include <cstdint>
#include <iomanip>
#include <ostream>
#include <tuple>
#include <unordered_map>

namespace thes {
namespace fmt {
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

struct Style {
  Colour fg = Colour::NONE;
  Colour bg = Colour::NONE;
  bool bold : 1 = false;
  bool faint : 1 = false;
  bool italic : 1 = false;
  bool underline : 1 = false;
  bool blinking : 1 = false;
  bool reversed : 1 = false;
  bool concealed : 1 = false;
  bool crossed_out : 1 = false;

  friend std::ostream& operator<<(std::ostream& s, Style style) {
    s << "\033[m";

    switch (style.fg) {
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

    switch (style.bg) {
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

    if (style.bold) {
      s << "\033[1m";
    }
    if (style.faint) {
      s << "\033[2m";
    }
    if (style.italic) {
      s << "\033[3m";
    }
    if (style.underline) {
      s << "\033[4m";
    }
    if (style.blinking) {
      s << "\033[5m";
    }
    if (style.reversed) {
      s << "\033[7m";
    }
    if (style.concealed) {
      s << "\033[8m";
    }
    if (style.crossed_out) {
      s << "\033[9m";
    }
    return s;
  }

  constexpr bool operator==(const Style& s) const = default;
};

struct Foreground {
  Colour colour;

  constexpr explicit Foreground(Colour c) : colour(c) {}

  void apply(Style& s) const {
    s.fg = colour;
  }
};
struct Background {
  Colour colour;

  constexpr explicit Background(Colour c) : colour(c) {}

  void apply(Style& s) const {
    s.bg = colour;
  }
};

#define THES_DEFINE_STYLE_APPLIER(STRUCT_NAME, ATTRIBUTE_NAME) \
  template<bool tApplyOrInvert> \
  struct STRUCT_NAME { \
    void apply(Style& s) const { \
      s.ATTRIBUTE_NAME = tApplyOrInvert; \
    } \
  }

THES_DEFINE_STYLE_APPLIER(Bold, bold);
THES_DEFINE_STYLE_APPLIER(Faint, faint);
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

inline constexpr Bold<true> bold{};
inline constexpr Faint<true> faint{};
inline constexpr Italic<true> italic{};
inline constexpr Underline<true> underline{};
inline constexpr Blinking<true> blinking{};
inline constexpr Reversed<true> reversed{};
inline constexpr Concealed<true> concealed{};
inline constexpr CrossedOut<true> crossed_out{};

inline constexpr Bold<false> not_bold{};
inline constexpr Faint<false> not_faint{};
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
    if (style != previous) {
      stream << previous;
    }

    if (previous != Style{}) {
      style = previous;
    } else {
      remove(stream);
    }
  }

  template<StyleApplier TApp>
  void set(const TApp& app) {
    Style& style = get(stream);
    app.apply(style);
    stream << style;
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

struct PrecisionManip {
  int precision;

  void apply(std::ostream& stream) const {
    stream << std::setprecision(precision);
  }
};
struct FixedFloatManip {
  static void apply(std::ostream& stream) {
    stream << std::fixed;
  }
};
struct ScientificFloatManip {
  static void apply(std::ostream& stream) {
    stream << std::scientific;
  }
};
struct WidthManip {
  int width;

  void apply(std::ostream& stream) const {
    stream << std::setw(width);
  }
};
struct FillManip {
  char fill;

  void apply(std::ostream& stream) const {
    stream << std::setfill(fill);
  }
};
struct InternalAdjustmentManip {
  static void apply(std::ostream& stream) {
    stream << std::internal;
  }
};

template<typename T>
inline constexpr PrecisionManip full_precision{std::numeric_limits<T>::digits10};
inline constexpr FixedFloatManip fixed{};
inline constexpr ScientificFloatManip scientific{};
inline constexpr InternalAdjustmentManip internal{};

inline constexpr PrecisionManip precision(int precision) {
  return PrecisionManip{precision};
}
inline constexpr WidthManip width(int width) {
  return WidthManip{width};
}
inline constexpr FillManip fill(char fill) {
  return FillManip{fill};
}

template<typename TApp>
concept StreamApplier = requires(const TApp& app, std::ostream& s) {
  { app.apply(s) } -> std::same_as<void>;
};

template<typename TApp>
concept Applier = StyleApplier<TApp> || StreamApplier<TApp>;

struct FormatContext {
  using FmtFlags = typename std::ios_base::fmtflags;
  using StreamSize = std::streamsize;

  StyleContext style;
  FmtFlags flags;
  StreamSize precision;
  StreamSize width;

  explicit FormatContext(std::ostream& s)
      : style(s), flags(s.flags()), precision(s.precision()), width(s.width()) {}
  FormatContext(const FormatContext&) = delete;
  FormatContext(FormatContext&&) = delete;
  FormatContext& operator=(const FormatContext&) = delete;
  FormatContext& operator=(FormatContext&&) = delete;
  ~FormatContext() {
    std::ostream& stream = style.stream;
    stream.flags(flags);
    stream.precision(precision);
    stream.width(width);
  }

  template<Applier TApp>
  void set(const TApp& app) {
    if constexpr (StyleApplier<TApp>) {
      style.set(app);
    }
    if constexpr (StreamApplier<TApp>) {
      app.apply(style.stream);
    }
  }
};

inline FormatContext make_context(std::ostream& s) {
  return FormatContext{s};
}

template<Applier TApp1, Applier TApp2>
struct MergedStyleApplier {
  TApp1 app1;
  TApp2 app2;

  void apply(Style& s) const
  requires StyleApplier<TApp1> || StyleApplier<TApp2>
  {
    if constexpr (StyleApplier<TApp1>) {
      app1.apply(s);
    }
    if constexpr (StyleApplier<TApp2>) {
      app2.apply(s);
    }
  }

  void apply(std::ostream& s) const
  requires StreamApplier<TApp1> || StreamApplier<TApp2>
  {
    if constexpr (StreamApplier<TApp1>) {
      app1.apply(s);
    }
    if constexpr (StreamApplier<TApp2>) {
      app2.apply(s);
    }
  }
};

template<Applier TApp1, Applier TApp2>
inline constexpr MergedStyleApplier<TApp1, TApp2> operator|(TApp1&& app1, TApp2&& app2) {
  return {std::forward<TApp1>(app1), std::forward<TApp2>(app2)};
}

template<Applier TApp, typename... TArgs>
struct FormattedArgs {
  TApp applier;
  std::tuple<TArgs...> arguments;

  explicit FormattedArgs(TApp&& app, TArgs&&... args)
      : applier{std::forward<TApp>(app)}, arguments{std::forward<TArgs>(args)...} {}

  friend std::ostream& operator<<(std::ostream& s, const FormattedArgs& styled) {
    FormatContext ctx{s};
    ctx.set(styled.applier);
    std::apply([&](const auto&... args) { (s << ... << args); }, styled.arguments);
    return s;
  }
};

inline constexpr auto zero_pad(int padding) {
  return width(padding) | fill('0') | internal;
}
} // namespace fmt

template<fmt::Applier TApp, typename... TArgs>
inline fmt::FormattedArgs<TApp, TArgs...> formatted(TApp&& app, TArgs&&... args) {
  return fmt::FormattedArgs<TApp, TArgs...>{std::forward<TApp>(app), std::forward<TArgs>(args)...};
}
} // namespace thes

#endif // INCLUDE_THESAUROS_FORMAT_FORMAT_HPP
