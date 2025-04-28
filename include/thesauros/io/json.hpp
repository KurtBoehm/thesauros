#ifndef INCLUDE_THESAUROS_IO_JSON_HPP
#define INCLUDE_THESAUROS_IO_JSON_HPP

#include <concepts>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "thesauros/concepts/type-traits.hpp"
#include "thesauros/io/delimiter.hpp"
#include "thesauros/macropolis/enum.hpp"
#include "thesauros/macropolis/serial-value.hpp"
#include "thesauros/macropolis/type.hpp"
#include "thesauros/ranges/concepts.hpp"
#include "thesauros/utility/numeric-string.hpp"
#include "thesauros/static-ranges.hpp"
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
  auto output(auto it) const {
    if (state_.has_value()) {
      const auto depth = state_->depth * state_->step;
      for (std::size_t i = 0; i < depth; ++i) {
        *it++ = ' ';
      }
    }
    return it;
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
inline auto write_json(auto it, T&& value, Indentation indent = {}) {
  using Type = std::remove_cvref_t<T>;
  return JsonWriter<Type>::write(it, std::forward<T>(value), indent);
}

template<typename T>
struct JsonPrinter {
  explicit JsonPrinter(T&& value, Indentation indent = {})
      : value_(std::forward<T>(value)), indent_{indent} {}

  [[nodiscard]] const T& value() const {
    return value_;
  }
  [[nodiscard]] const Indentation& indent() const {
    return indent_;
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
  static auto write(auto it, const bool value, Indentation /*indent*/ = {}) {
    const std::string_view str = value ? "true" : "false";
    return std::copy(str.begin(), str.end(), it);
  }
};
template<typename TNum>
requires std::integral<TNum> || std::floating_point<TNum>
struct JsonWriter<TNum> {
  static auto write(auto it, const TNum value, Indentation /*indent*/ = {}) {
    const auto str = numeric_string(value).value();
    return std::copy(str.begin(), str.end(), it);
  }
};

template<CharacterType TChar>
struct JsonWriter<std::basic_string_view<TChar>> {
  static auto write(auto it, std::string_view value, Indentation /*indent*/ = {}) {
    *it++ = '"';
    it = escape_string(value, it);
    *it++ = '"';
    return it;
  }
};
template<CharacterType TChar, std::size_t tSize>
struct JsonWriter<TChar[tSize]> : public JsonWriter<std::basic_string_view<TChar>> {};
template<CharacterType TChar>
struct JsonWriter<TChar*> : public JsonWriter<std::basic_string_view<TChar>> {};
template<CharacterType TChar>
struct JsonWriter<const TChar*> : public JsonWriter<std::basic_string_view<TChar>> {};
template<CharacterType TChar>
struct JsonWriter<std::basic_string<TChar>> : public JsonWriter<std::basic_string_view<TChar>> {};

template<typename T>
struct JsonWriter<std::optional<T>> {
  using Opt = std::optional<T>;
  static auto write(auto it, const Opt& opt, Indentation indent = {}) {
    if (opt.has_value()) {
      return JsonWriter<T>::write(it, *opt, indent);
    }
    const std::string_view str{"null"};
    return std::copy(str.begin(), str.end(), it);
  }
};
template<>
struct JsonWriter<std::filesystem::path> {
  using Path = std::filesystem::path;
  static auto write(auto it, const Path& p, Indentation indent = {}) {
    return JsonWriter<Path::string_type>::write(it, p.native(), indent);
  }
};

template<MapRange TMap>
struct JsonWriter<TMap> {
  static auto write(auto it, const TMap& map, Indentation indent = {}) {
    const auto indent1 = indent + 1;

    *it++ = '{';
    *it++ = indent.separator();

    for (Delimiter delim{","}; const auto& [k, v] : map) {
      delim.output(it, indent1.separator());
      it = indent1.output(it);

      *it++ = '"';
      it = escape_string(k, it);
      *it++ = '"';
      *it++ = ':';
      *it++ = ' ';

      it = write_json(it, serial_value(v), indent1);
    }

    *it++ = indent1.separator();
    it = indent.output(it);
    *it++ = '}';

    return it;
  }
};

template<typename TRange>
requires(AnyRange<TRange> && !MapRange<TRange>)
struct JsonWriter<TRange> {
  static auto write(auto it, const TRange& rng, Indentation indent = {}) {
    const auto indent1 = indent + 1;

    *it++ = '[';
    indent.reduced_separator([&](auto c) { *it++ = c; });

    for (Delimiter delim{","}; const auto& v : rng) {
      delim.output(it, indent1.separator());
      it = indent1.output(it);
      it = write_json(it, serial_value(v), indent1);
    }

    indent.reduced_separator([&](auto c) { *it++ = c; });
    it = indent.output(it);
    *it++ = ']';

    return it;
  }
};

template<HasTypeInfo T>
struct JsonWriter<T> {
  static auto write(auto it, const T& value, Indentation indent = {}) {
    using Info = TypeInfo<T>;
    const auto indent1 = indent + 1;

    *it++ = '{';
    *it++ = indent.separator();

    constexpr std::size_t static_size = std::tuple_size_v<decltype(Info::static_members)>;
    constexpr std::size_t size = static_size + std::tuple_size_v<decltype(Info::members)>;

    star::iota<0, size> | star::for_each([&](auto i) {
      constexpr bool is_static = i < static_size;
      constexpr auto member = [&] {
        if constexpr (is_static) {
          return star::get_at<i>(Info::static_members);
        } else {
          return star::get_at<i - static_size>(Info::members);
        }
      }();
      using Member = decltype(member);

      it = indent1.output(it);
      *it++ = '"';
      it = escape_string(Member::serial_name.view(), it);
      *it++ = '"';
      *it++ = ':';
      *it++ = ' ';

      if constexpr (is_static) {
        it = write_json(it, serial_value(Member::value), indent1);
      } else {
        it = write_json(it, serial_value(value.*Member::pointer), indent1);
      }
      if constexpr (i + 1 < size) {
        *it++ = ',';
      }

      *it++ = indent1.separator();
    });

    it = indent.output(it);
    *it++ = '}';

    return it;
  }
};

template<HasEnumInfo TEnum>
struct JsonWriter<TEnum> {
  using Path = std::filesystem::path;
  template<typename TIt>
  static auto write(TIt it, TEnum value, Indentation indent = {}) {
    using EnumInfo = ::thes::EnumInfo<TEnum>;

    auto impl = [&]<std::size_t tHead, std::size_t... tTail>(
                  auto rec, std::index_sequence<tHead, tTail...>) -> TIt {
      constexpr auto value_info = star::get_at<tHead>(EnumInfo::values);
      if (value_info.value == value) {
        return JsonWriter<std::string_view>::write(it, value_info.serial_name.view(), indent);
      }
      if constexpr (sizeof...(tTail) > 0) {
        return rec(rec, std::index_sequence<tTail...>{});
      } else {
        using Under = std::underlying_type_t<TEnum>;
        return JsonWriter<Under>::write(it, static_cast<Under>(value), indent);
      }
    };
    return impl(impl, std::make_index_sequence<std::tuple_size_v<decltype(EnumInfo::values)>>{});
  }
};
} // namespace thes

#endif // INCLUDE_THESAUROS_IO_JSON_HPP
