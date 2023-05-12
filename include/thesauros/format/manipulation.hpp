#ifndef INCLUDE_THESAUROS_FORMAT_MANIPULATION_HPP
#define INCLUDE_THESAUROS_FORMAT_MANIPULATION_HPP

#include <iomanip>
#include <ios>
#include <type_traits>

#include "thesauros/io/concepts.hpp"

namespace thes::fmt {
template<typename T>
struct IsIoManipTrait : public std::false_type {};
template<typename T>
inline constexpr bool is_io_manip = IsIoManipTrait<T>::value;

struct StreamState {
  using FmtFlags = typename std::ios_base::fmtflags;
  using StreamSize = std::streamsize;

  explicit StreamState(const std::ios_base& ios)
      : flags_(ios.flags()), precision_(ios.precision()), width_(ios.width()) {}

  void restore(std::ios_base& ios) const {
    ios.flags(flags_);
    ios.precision(precision_);
    ios.width(width_);
  }

private:
  FmtFlags flags_;
  StreamSize precision_;
  StreamSize width_;
};

struct PrecisionManip {
  int precision;

  void apply(std::ostream& stream) const {
    stream << std::setprecision(precision);
  }
};
template<>
struct IsIoManipTrait<PrecisionManip> : public std::true_type {};

inline constexpr PrecisionManip precision(int precision) {
  return PrecisionManip{precision};
}
template<typename T>
inline constexpr PrecisionManip digit_precision() {
  return PrecisionManip{std::numeric_limits<T>::digits10};
}

struct FixedFloatManip {
  static void apply(std::ostream& stream) {
    stream << std::fixed;
  }
};
template<>
struct IsIoManipTrait<FixedFloatManip> : public std::true_type {};

inline constexpr FixedFloatManip fixed() {
  return FixedFloatManip{};
}

struct ScientificFloatManip {
  static void apply(std::ostream& stream) {
    stream << std::scientific;
  }
};
template<>
struct IsIoManipTrait<ScientificFloatManip> : public std::true_type {};

inline constexpr ScientificFloatManip scientific() {
  return ScientificFloatManip{};
}

struct WidthManip {
  int width;

  void apply(std::ostream& stream) const {
    stream << std::setw(width);
  }
};
template<>
struct IsIoManipTrait<WidthManip> : public std::true_type {};

inline constexpr WidthManip width(int width) {
  return WidthManip{width};
}

struct FillManip {
  char fill;

  void apply(std::ostream& stream) const {
    stream << std::setfill(fill);
  }
};
template<>
struct IsIoManipTrait<FillManip> : public std::true_type {};

inline constexpr FillManip fill(char fill) {
  return FillManip{fill};
}

struct InternalAdjustmentManip {
  static void apply(std::ostream& stream) {
    stream << std::internal;
  }
};
template<>
struct IsIoManipTrait<InternalAdjustmentManip> : public std::true_type {};

inline constexpr InternalAdjustmentManip internal() {
  return InternalAdjustmentManip{};
}

template<typename... TManips>
struct ManipCombine {
  constexpr explicit ManipCombine(TManips&&... ms) : manips(std::forward<TManips>(ms)...) {}

  void apply(std::ostream& stream) const {
    std::apply([&stream](const TManips&... args) { (..., args.apply(stream)); }, manips);
  }

  std::tuple<TManips...> manips;
};
template<typename TMan1, typename TMan2>
ManipCombine(const TMan1&, const TMan2&) -> ManipCombine<const TMan1&, const TMan2&>;

template<typename T>
struct IsManipTrait : public IsIoManipTrait<T> {};
template<typename... TManips>
struct IsManipTrait<ManipCombine<TManips...>> : public std::true_type {};
template<typename T>
inline constexpr bool is_manip = IsManipTrait<T>::value;

template<typename TMan1, typename TMan2>
requires is_io_manip<std::decay_t<TMan1>> && is_io_manip<std::decay_t<TMan2>>
inline constexpr auto operator|(TMan1&& man1, TMan2&& man2) {
  return ManipCombine<TMan1, TMan2>{std::forward<TMan1>(man1), std::forward<TMan2>(man2)};
}

template<typename... TManips, typename TManip>
requires is_io_manip<std::decay_t<TManip>>
inline constexpr auto operator|(ManipCombine<TManips...>&& man1, TManip&& man2) {
  return std::apply(
    [&man2](auto&&... args) {
      return ManipCombine<TManips..., TManip>{std::forward<TManips>(args)...,
                                              std::forward<TManip>(man2)};
    },
    std::move(man1.manips));
}
template<typename... TManips, typename TManip>
requires is_io_manip<std::decay_t<TManip>>
inline constexpr auto operator|(const ManipCombine<TManips...>& man1, TManip&& man2) {
  return ManipCombine(man1) | std::forward<TManip>(man2);
}

template<typename TManip, typename... TManips>
requires is_io_manip<std::decay_t<TManip>>
inline constexpr auto operator|(TManip&& man1, ManipCombine<TManips...>&& man2) {
  return std::apply(
    [&man1](auto&&... args) {
      return ManipCombine<TManip, TManips...>{std::forward<TManip>(man1),
                                              std::forward<TManips>(args)...};
    },
    std::move(man2.manips));
}
template<typename TManip, typename... TManips>
requires is_io_manip<std::decay_t<TManip>>
inline constexpr auto operator|(TManip&& man1, const ManipCombine<TManips...>& man2) {
  return std::forward<TManip>(man1) | ManipCombine(man2);
}

template<typename... TManips1, typename... TManips2>
inline constexpr auto operator|(ManipCombine<TManips1...>&& man1,
                                ManipCombine<TManips2...>&& man2) {
  return std::apply(
    [&man2](TManips1... args1) {
      return std::apply(
        [&args1...](TManips2... args2) {
          return ManipCombine<TManips1..., TManips2...>{std::forward<TManips1>(args1)...,
                                                        std::forward<TManips2>(args2)...};
        },
        std::move(man2.manips));
    },
    std::move(man1.manips));
}
template<typename... TManips1, typename... TManips2>
inline constexpr auto operator|(const ManipCombine<TManips1...>& man1,
                                ManipCombine<TManips2...>&& man2) {
  return ManipCombine{man1} | std::forward<ManipCombine<TManips2...>>(man2);
}
template<typename... TManips1, typename... TManips2>
inline constexpr auto operator|(ManipCombine<TManips1...>&& man1,
                                const ManipCombine<TManips2...>& man2) {
  return std::forward<ManipCombine<TManips1...>>(man1) | ManipCombine{man2};
}
template<typename... TManips1, typename... TManips2>
inline constexpr auto operator|(const ManipCombine<TManips1...>& man1,
                                const ManipCombine<TManips2...>& man2) {
  return ManipCombine{man1} | ManipCombine{man2};
}

inline constexpr auto zero_pad(int padding) {
  return width(padding) | fill('0') | internal();
}

template<OStream TStream, typename TManip>
struct ManipStream {
  ManipStream(TStream& stream, const TManip& manip)
      : stream_(stream), state_(stream), manip_(manip) {
    StreamState state(stream_);
    manip_.apply(stream_);
  }
  ManipStream(const ManipStream&) = delete;
  ManipStream(ManipStream&&) = delete;
  ManipStream& operator=(const ManipStream&) = delete;
  ManipStream& operator=(ManipStream&&) = delete;
  ~ManipStream() {
    state_.restore(stream_);
  }

  template<typename T>
  ManipStream& operator<<(const T& t) {
    stream_ << t;
    return *this;
  }
  ManipStream& operator<<(std::ios_base& (*func)(std::ios_base&)) {
    stream_ << func;
    return *this;
  }
  ManipStream& operator<<(std::ios& (*func)(std::ios&)) {
    stream_ << func;
    return *this;
  }
  ManipStream& operator<<(std::ostream& (*func)(std::ostream&)) {
    stream_ << func;
    return *this;
  }

private:
  TStream& stream_;
  StreamState state_;
  const TManip& manip_;
};

template<typename TManip, typename... TArgs>
struct ArgsManipulated {
  explicit ArgsManipulated(TManip&& manip, TArgs&&... args)
      : args_(std::forward<TArgs>(args)...), man_(std::forward<TManip>(manip)) {}

  template<OStream TStream>
  friend std::ostream& operator<<(TStream& stream, const ArgsManipulated& mana) {
    ManipStream<TStream, TManip> man_stream(stream, mana.man_);
    std::apply([&man_stream](const auto&... args) { (man_stream << ... << args); }, mana.args_);
    return stream;
  }

private:
  std::tuple<TArgs...> args_;
  TManip man_;
};

template<typename TManip, typename... TArgs>
inline constexpr ArgsManipulated<TManip, TArgs...> manip_args(TManip&& manip, TArgs&&... args) {
  return ArgsManipulated<TManip, TArgs...>{std::forward<TManip>(manip),
                                           std::forward<TArgs>(args)...};
}
} // namespace thes::fmt

#endif // INCLUDE_THESAUROS_FORMAT_MANIPULATION_HPP
