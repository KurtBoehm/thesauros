#ifndef INCLUDE_THESAUROS_IO_STYLE_HPP
#define INCLUDE_THESAUROS_IO_STYLE_HPP

#include <cstddef>
#include <ostream>

namespace thes {
namespace ansi {
struct EscSeq {
  template<std::size_t tSize>
  constexpr explicit EscSeq(const char (&string)[tSize]) : sequence_(string, tSize) {}
  EscSeq(const EscSeq&) = delete;
  EscSeq(EscSeq&&) = delete;
  EscSeq& operator=(const EscSeq&) = delete;
  EscSeq& operator=(EscSeq&&) = delete;
  ~EscSeq() = default;

  void apply(std::ostream& stream) const {
    stream << sequence_;
  }

private:
  std::string_view sequence_;
};
} // namespace ansi

inline constexpr ansi::EscSeq fg_black{"30"};
inline constexpr ansi::EscSeq fg_red{"31"};
inline constexpr ansi::EscSeq fg_green{"32"};
inline constexpr ansi::EscSeq fg_yellow{"33"};
inline constexpr ansi::EscSeq fg_blue{"34"};
inline constexpr ansi::EscSeq fg_magenta{"35"};
inline constexpr ansi::EscSeq fg_cyan{"36"};
inline constexpr ansi::EscSeq fg_white{"37"};
inline constexpr ansi::EscSeq fg_bright_black{"90"};
inline constexpr ansi::EscSeq fg_bright_red{"91"};
inline constexpr ansi::EscSeq fg_bright_green{"92"};
inline constexpr ansi::EscSeq fg_bright_yellow{"93"};
inline constexpr ansi::EscSeq fg_bright_blue{"94"};
inline constexpr ansi::EscSeq fg_bright_magenta{"95"};
inline constexpr ansi::EscSeq fg_bright_cyan{"96"};
inline constexpr ansi::EscSeq fg_bright_white{"97"};

inline constexpr ansi::EscSeq bg_black{"40"};
inline constexpr ansi::EscSeq bg_red{"41"};
inline constexpr ansi::EscSeq bg_green{"42"};
inline constexpr ansi::EscSeq bg_yellow{"43"};
inline constexpr ansi::EscSeq bg_blue{"44"};
inline constexpr ansi::EscSeq bg_magenta{"45"};
inline constexpr ansi::EscSeq bg_cyan{"46"};
inline constexpr ansi::EscSeq bg_white{"47"};
inline constexpr ansi::EscSeq bg_bright_black{"100"};
inline constexpr ansi::EscSeq bg_bright_red{"101"};
inline constexpr ansi::EscSeq bg_bright_green{"102"};
inline constexpr ansi::EscSeq bg_bright_yellow{"103"};
inline constexpr ansi::EscSeq bg_bright_blue{"104"};
inline constexpr ansi::EscSeq bg_bright_magenta{"105"};
inline constexpr ansi::EscSeq bg_bright_cyan{"106"};
inline constexpr ansi::EscSeq bg_bright_white{"107"};

inline constexpr ansi::EscSeq bold{"1"};
inline constexpr ansi::EscSeq faint{"2"};
inline constexpr ansi::EscSeq italic{"3"};
inline constexpr ansi::EscSeq underline{"4"};
// this is “slow blink” according to Wikipedia
inline constexpr ansi::EscSeq blinking{"5"};
// “rapid blink” is left out
inline constexpr ansi::EscSeq reversed{"7"};
inline constexpr ansi::EscSeq concealed{"8"};
inline constexpr ansi::EscSeq crossed_out{"9"};

namespace ansi {
template<typename... TEscape>
struct EscCombine {
  constexpr explicit EscCombine(const TEscape&... es) : escapes(es...) {}

  void apply(std::ostream& stream) const {
    std::apply(
      [&stream](const TEscape&... args) {
        (Init{} << ... << Wrap<TEscape>{stream, args});
      },
      escapes);
  }

  std::tuple<const TEscape&...> escapes;

private:
  template<typename TEsc>
  struct Wrap {
    std::ostream& stream;
    const TEsc& escape;
  };
  struct Inner {
    template<typename TEsc>
    friend Inner operator<<(Inner /*init*/, Wrap<TEsc> wrap) {
      wrap.stream << ";";
      wrap.escape.apply(wrap.stream);
      return Inner{};
    }
  };
  struct Init {
    template<typename TEsc>
    friend Inner operator<<(Init /*init*/, Wrap<TEsc> wrap) {
      wrap.escape.apply(wrap.stream);
      return Inner{};
    }
  };
};

template<typename... TEscs>
EscCombine(const TEscs&...) -> EscCombine<const TEscs&...>;

// Only the pre-defined constants are supposed to be used, which is why no overloads
// for rvalue references are defined.
inline constexpr auto operator|(const EscSeq& esc1, const EscSeq& esc2) {
  return EscCombine{esc1, esc2};
}
template<typename... TEscs>
inline constexpr auto operator|(const EscSeq& esc1, const EscCombine<TEscs...>& esc2) {
  return std::apply(
    [&esc1](const TEscs&... args) {
      return EscCombine<EscSeq, TEscs...>{esc1, args...};
    },
    esc2.escapes);
}
template<typename... TEscs>
inline constexpr auto operator|(const EscCombine<TEscs...>& esc1, const EscSeq& esc2) {
  return std::apply(
    [&esc2](const TEscs&... args) {
      return EscCombine<TEscs..., EscSeq>{args..., esc2};
    },
    esc1.escapes);
}
template<typename... TEscs1, typename... TEscs2>
inline constexpr auto operator|(const EscCombine<TEscs1...>& esc1,
                                const EscCombine<TEscs2...>& esc2) {
  return std::apply(
    [&esc2](const TEscs1&... args1) {
      return std::apply(
        [&args1...](const TEscs2&... args2) {
          return EscCombine<TEscs1..., TEscs2...>{args1..., args2...};
        },
        esc2.escapes);
    },
    esc1.escapes);
}

template<typename T>
struct IsEscapeTrait : public std::false_type {};
template<typename T>
inline constexpr bool is_escape = IsEscapeTrait<T>::value;

template<>
struct IsEscapeTrait<EscSeq> : public std::true_type {};
template<typename... TEscs>
struct IsEscapeTrait<EscCombine<TEscs...>> : public std::true_type {};
} // namespace ansi

template<typename TEsc, typename... TArgs>
struct ArgsStyled {
  template<typename TOEsc, typename... TOArgs>
  friend struct ArgsStyled;

  explicit ArgsStyled(TEsc&& esc, TArgs&&... args)
      : args_(std::forward<TArgs>(args)...), esc_(std::forward<TEsc>(esc)) {}

  std::tuple<TArgs...> args_;
  TEsc esc_;
};

template<typename TEscape, typename... TEscapes>
struct ArgsStream {
  using Tuple = std::tuple<const TEscapes&...>;

  ArgsStream(std::ostream& stream, const TEscape& escape, const TEscapes&... escapes)
      : stream_(stream), escape_(escape), escapes_{escapes...} {
    stream << "\033[";
    escape_.apply(stream);
    stream << "m";
  }
  ArgsStream(const ArgsStream&) = delete;
  ArgsStream(ArgsStream&&) = delete;
  ArgsStream& operator=(const ArgsStream&) = delete;
  ArgsStream& operator=(ArgsStream&&) = delete;
  ~ArgsStream() {
    stream_ << "\033[m\033[";
    if constexpr (sizeof...(TEscapes) > 0) {
      std::apply([&stream = stream_](const TEscapes&... escapes) { (... | escapes).apply(stream); },
                 escapes_);
    }
    stream_ << "m";
  }

  template<typename T>
  ArgsStream& operator<<(const T& t) {
    stream_ << t;
    return *this;
  }
  template<typename TEsc, typename... TArgs>
  ArgsStream& operator<<(const ArgsStyled<TEsc, TArgs...>& value) {
    std::apply(
      [&escape = escape_, &stream = stream_, &value](const TEscapes&... escapes) {
        ArgsStream<TEsc, TEscapes..., TEscape> inner_stream(stream, value.esc_, escapes..., escape);
        std::apply([&inner_stream](const TArgs&... args) { (inner_stream << ... << args); },
                   value.args_);
      },
      escapes_);
    return *this;
  }
  ArgsStream& operator<<(std::ios_base& (*func)(std::ios_base&)) {
    stream_ << func;
    return *this;
  }
  ArgsStream& operator<<(std::ios& (*func)(std::ios&)) {
    stream_ << func;
    return *this;
  }
  ArgsStream& operator<<(std::ostream& (*func)(std::ostream&)) {
    stream_ << func;
    return *this;
  }

private:
  std::ostream& stream_;
  const TEscape& escape_;
  Tuple escapes_;
};

template<typename TEsc, typename... TArgs>
std::ostream& operator<<(std::ostream& stream, const ArgsStyled<TEsc, TArgs...>& value) {
  ArgsStream<TEsc> inner_stream(stream, value.esc_);
  std::apply([&inner_stream](const TArgs&... args) { (inner_stream << ... << args); }, value.args_);
  return stream;
}

template<typename TEsc, typename... TArgs>
inline constexpr ArgsStyled<TEsc, TArgs...> styled_args(TEsc&& esc, TArgs&&... args) {
  return ArgsStyled<TEsc, TArgs...>{std::forward<TEsc>(esc), std::forward<TArgs>(args)...};
}
} // namespace thes

#endif // INCLUDE_THESAUROS_IO_STYLE_HPP
