#ifndef INCLUDE_THESAUROS_FORMAT_FORMAT_HPP
#define INCLUDE_THESAUROS_FORMAT_FORMAT_HPP

#include <concepts>
#include <ios>
#include <ostream>
#include <tuple>

#include "thesauros/format/manip.hpp"
#include "thesauros/format/style.hpp"

namespace thes {
namespace fmt {
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
