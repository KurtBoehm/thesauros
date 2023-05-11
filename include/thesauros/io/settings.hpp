#ifndef INCLUDE_THESAUROS_IO_SETTINGS_HPP
#define INCLUDE_THESAUROS_IO_SETTINGS_HPP

#include "thesauros/io/manipulation.hpp"
#include "thesauros/io/style.hpp"

namespace thes {
template<typename TEsc, typename TManip>
struct StreamSettingCombine;

template<typename... TTEscs, typename... TManips>
struct StreamSettingCombine<ansi::EscCombine<TTEscs...>, ManipCombine<TManips...>> {
  using TEscs = ansi::EscCombine<TTEscs...>;
  using Manips = ManipCombine<TManips...>;

  template<typename TEsc, typename TMan>
  StreamSettingCombine(TEsc&& es, TMan&& ms)
      : escapes(std::forward<TEsc>(es)), manips(std::forward<TMan>(ms)) {}

  TEscs escapes;
  Manips manips;
};

template<typename... TEscs, typename... TManips>
StreamSettingCombine(ansi::EscCombine<TEscs...>, ManipCombine<TManips...>)
  -> StreamSettingCombine<ansi::EscCombine<TEscs...>, ManipCombine<TManips...>>;

template<typename... TEscs, typename... TManips, typename TEsc>
requires ansi::is_escape<std::decay_t<TEsc>>
inline constexpr auto
operator|(const StreamSettingCombine<ansi::EscCombine<TEscs...>, ManipCombine<TManips...>>& ssc,
          TEsc&& esc) {
  return StreamSettingCombine{ssc.escapes | std::forward<TEsc>(esc), ssc.manips};
}
template<typename... TEscs, typename... TManips, typename TEsc>
requires ansi::is_escape<std::decay_t<TEsc>>
inline constexpr auto
operator|(StreamSettingCombine<ansi::EscCombine<TEscs...>, ManipCombine<TManips...>>&& ssc,
          TEsc&& esc) {
  return StreamSettingCombine{std::move(ssc.escapes) | std::forward<TEsc>(esc),
                              std::move(ssc.manips)};
}

template<typename... TEscs, typename... TManips, typename TManip>
requires is_manip<std::decay_t<TManip>>
inline constexpr auto
operator|(const StreamSettingCombine<ansi::EscCombine<TEscs...>, ManipCombine<TManips...>>& ssc,
          TManip&& manip) {
  return StreamSettingCombine{ssc.escapes, ssc.manips | std::forward<TManip>(manip)};
}
template<typename... TEscs, typename... TManips, typename TManip>
requires is_manip<std::decay_t<TManip>>
inline constexpr auto
operator|(StreamSettingCombine<ansi::EscCombine<TEscs...>, ManipCombine<TManips...>>&& ssc,
          TManip&& manip) {
  return StreamSettingCombine{std::move(ssc.escapes),
                              std::move(ssc.manips) | std::forward<TManip>(manip)};
}

template<typename TEsc, typename TManip>
requires ansi::is_escape<std::decay_t<TEsc>> && is_manip<std::decay_t<TManip>>
inline constexpr auto operator|(TEsc&& esc, TManip&& manip) {
  return StreamSettingCombine{ansi::EscCombine{std::forward<TEsc>(esc)},
                              ManipCombine{std::forward<TManip>(manip)}};
}
template<typename TManip, typename TEsc>
requires is_manip<std::decay_t<TManip>> && ansi::is_escape<std::decay_t<TEsc>>
inline constexpr auto operator|(TManip&& manip, TEsc&& esc) {
  return StreamSettingCombine{ansi::EscCombine{std::forward<TEsc>(esc)},
                              ManipCombine{std::forward<TManip>(manip)}};
}

template<typename... TEscs, typename... TManips, typename... TArgs>
inline constexpr auto
set(const StreamSettingCombine<ansi::EscCombine<TEscs...>, ManipCombine<TManips...>>& ssc,
    TArgs&&... args) {
  return styled_args(ssc.escapes, manip_args(ssc.manips, std::forward<TArgs>(args)...));
}
template<typename TEsc, typename... TArgs>
requires ansi::is_escape<std::decay_t<TEsc>>
inline constexpr auto set(TEsc&& esc, TArgs&&... args) {
  return styled_args(std::forward<TEsc>(esc), std::forward<TArgs>(args)...);
}
template<typename TMan, typename... TArgs>
requires is_manip<std::decay_t<TMan>>
inline constexpr auto set(TMan&& man, TArgs&&... args) {
  return manip_args(std::forward<TMan>(man), std::forward<TArgs>(args)...);
}

template<typename... TArgs>
struct ArgsPrinter {
  explicit ArgsPrinter(TArgs&&... args) : args_(std::forward<TArgs>(args)...) {}

  friend std::ostream& operator<<(std::ostream& stream, const ArgsPrinter& ap) {
    std::apply([&stream](const TArgs&... args) { (stream << ... << args); }, ap.args_);
    return stream;
  }

private:
  std::tuple<TArgs...> args_;
};

struct UseStyleType {
  bool value;
  constexpr explicit UseStyleType(bool v) : value{v} {}
};
inline constexpr UseStyleType with_style{true};
inline constexpr UseStyleType no_style{false};

template<UseStyleType tUseStyle, typename TOpt, typename... TArgs>
inline constexpr auto set_opt(TOpt&& opt, TArgs&&... args) {
  if constexpr (tUseStyle.value) {
    return set(std::forward<TOpt>(opt), std::forward<TArgs>(args)...);
  } else {
    return ArgsPrinter<TArgs...>{std::forward<TArgs>(args)...};
  }
}
} // namespace thes

#endif // INCLUDE_THESAUROS_IO_SETTINGS_HPP
