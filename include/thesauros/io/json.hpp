#ifndef INCLUDE_THESAUROS_IO_JSON_HPP
#define INCLUDE_THESAUROS_IO_JSON_HPP

#include <concepts>
#include <cstddef>
#include <filesystem>
#include <iterator>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "thesauros/concepts/type-traits.hpp"
#include "thesauros/io/delimiter.hpp"
#include "thesauros/macropolis/members.hpp"
#include "thesauros/macropolis/serial-value.hpp"
#include "thesauros/ranges/concepts.hpp"
#include "thesauros/utility/numeric-string.hpp"
#include "thesauros/utility/static-ranges.hpp"
#include "thesauros/utility/string-escape.hpp"

namespace thes {
struct Indentation {
  constexpr Indentation() = default;
  explicit constexpr Indentation(std::size_t step) : Indentation{0, step} {}
  constexpr Indentation(std::size_t depth, std::size_t step) : state_{std::in_place, depth, step} {}

  Indentation& operator++() {
    if (state_.has_value()) {
      ++state_->depth;
    }
    return *this;
  }
  auto write_to(auto out_it) const {
    if (state_.has_value()) {
      const auto depth = state_->depth * state_->step;
      for (std::size_t i = 0; i < depth; ++i) {
        *out_it++ = ' ';
      }
    }
    return out_it;
  }

  friend std::ostream& operator<<(std::ostream& s, const Indentation& indent) {
    indent.write_to(std::ostream_iterator<char>{s});
    return s;
  }

  friend Indentation operator+(Indentation indent, std::size_t depth) {
    if (indent.state_.has_value()) {
      indent.state_->depth += depth;
    }
    return indent;
  }

  [[nodiscard]] char separator() const {
    return state_.has_value() ? '\n' : ' ';
  }

  void reduced_separator(auto op) const {
    if (state_.has_value()) {
      op('\n');
    }
  }

private:
  struct State {
    State(std::size_t p_depth, std::size_t p_step) : depth{p_depth}, step{p_step} {}

    std::size_t depth;
    std::size_t step;
  };
  std::optional<State> state_{std::nullopt};
};

// Indented on all lines apart from the first, which has to be indented by the caller.
template<typename T>
struct JsonWriter;

template<typename T>
inline auto write_json(auto out_it, T&& value, Indentation indent = {}) {
  using Type = std::remove_cvref_t<T>;
  return JsonWriter<Type>::write(out_it, std::forward<T>(value), indent);
}

template<typename T>
struct JsonPrinter {
  explicit JsonPrinter(T&& value, Indentation indent = {})
      : value_(std::forward<T>(value)), indent_{indent} {}

  friend std::ostream& operator<<(std::ostream& stream, const JsonPrinter& printer) {
    write_json(std::ostream_iterator<char>{stream}, printer.value_, printer.indent_);
    return stream;
  }

private:
  T value_;
  Indentation indent_;
};

template<typename T>
inline auto json_print(T&& value, Indentation indent = {}) {
  return JsonPrinter<T>{std::forward<T>(value), indent};
}

template<>
struct JsonWriter<bool> {
  static auto write(auto out_it, const bool value, Indentation /*indent*/ = {}) {
    const std::string_view str = value ? "true" : "false";
    return std::copy(str.begin(), str.end(), out_it);
  }
};
template<typename TNum>
requires std::integral<TNum> || std::floating_point<TNum>
struct JsonWriter<TNum> {
  static auto write(auto out_it, const TNum value, Indentation /*indent*/ = {}) {
    const auto str = numeric_string(value).value();
    return std::copy(str.begin(), str.end(), out_it);
  }
};

template<CharacterType TChar>
struct JsonWriter<std::basic_string_view<TChar>> {
  static auto write(auto out_it, std::string_view value, Indentation /*indent*/ = {}) {
    *out_it++ = '"';
    out_it = escape_string(value, out_it);
    *out_it++ = '"';
    return out_it;
  }
};
template<CharacterType TChar, std::size_t tSize>
struct JsonWriter<TChar[tSize]> : public JsonWriter<std::basic_string_view<TChar>> {};
template<CharacterType TChar>
struct JsonWriter<TChar*> : public JsonWriter<std::basic_string_view<TChar>> {};
template<CharacterType TChar>
struct JsonWriter<std::basic_string<TChar>> : public JsonWriter<std::basic_string_view<TChar>> {};

template<>
struct JsonWriter<std::filesystem::path> {
  using Path = std::filesystem::path;
  static auto write(auto out_it, const std::filesystem::path& p, Indentation indent = {}) {
    return JsonWriter<Path::string_type>::write(out_it, p.native(), indent);
  }
};

template<MapRange TMap>
struct JsonWriter<TMap> {
  static auto write(auto out_it, const TMap& map, Indentation indent = {}) {
    const auto indent1 = indent + 1;

    *out_it++ = '{';
    *out_it++ = indent.separator();

    for (Delimiter delim{","}; const auto& [k, v] : map) {
      delim.write_to(out_it, indent1.separator());
      out_it = indent1.write_to(out_it);

      *out_it++ = '"';
      out_it = escape_string(k, out_it);
      *out_it++ = '"';
      *out_it++ = ':';
      *out_it++ = ' ';

      out_it = write_json(out_it, serial_value(v), indent1);
    }

    *out_it++ = indent1.separator();
    out_it = indent.write_to(out_it);
    *out_it++ = '}';

    return out_it;
  }
};

template<typename TRange>
requires(AnyRange<TRange> && !MapRange<TRange>)
struct JsonWriter<TRange> {
  static auto write(auto out_it, const TRange& rng, Indentation indent = {}) {
    const auto indent1 = indent + 1;

    *out_it++ = '[';
    indent.reduced_separator([&](auto c) { *out_it++ = c; });

    for (Delimiter delim{","}; const auto& v : rng) {
      delim.write_to(out_it, indent1.separator());
      out_it = indent1.write_to(out_it);
      out_it = write_json(out_it, serial_value(v), indent1);
    }

    indent.reduced_separator([&](auto c) { *out_it++ = c; });
    out_it = indent.write_to(out_it);
    *out_it++ = ']';

    return out_it;
  }
};

template<typename T>
requires(requires { sizeof(TypeInfo<T>); })
struct JsonWriter<T> {
  static auto write(auto out_it, const T& value, Indentation indent = {}) {
    using Info = TypeInfo<T>;
    const auto indent1 = indent + 1;

    *out_it++ = '{';
    *out_it++ = indent.separator();

    constexpr std::size_t static_size = std::tuple_size_v<decltype(Info::static_members)>;
    constexpr std::size_t size = static_size + std::tuple_size_v<decltype(Info::members)>;

    star::iota<0, size> | star::for_each([&](auto i) {
      constexpr bool is_static = i < static_size;
      constexpr auto member = [&] {
        if constexpr (is_static) {
          return std::get<i>(Info::static_members);
        } else {
          return std::get<i - static_size>(Info::members);
        }
      }();
      using Member = decltype(member);

      out_it = indent1.write_to(out_it);
      *out_it++ = '"';
      out_it = escape_string(Member::serial_name.view(), out_it);
      *out_it++ = '"';
      *out_it++ = ':';
      *out_it++ = ' ';

      if constexpr (is_static) {
        out_it = write_json(out_it, serial_value(Member::value), indent1);
      } else {
        out_it = write_json(out_it, serial_value(value.*Member::pointer), indent1);
      }
      if constexpr (i + 1 < size) {
        *out_it++ = ',';
      }

      *out_it++ = indent1.separator();
    });

    out_it = indent.write_to(out_it);
    *out_it++ = '}';

    return out_it;
  }
};
} // namespace thes

#endif // INCLUDE_THESAUROS_IO_JSON_HPP
