#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_MAP_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_MAP_HPP

#include <functional>
#include <utility>

#include "thesauros/utility/static-ranges.hpp"
#include "thesauros/utility/static-string/static-string.hpp"
#include "thesauros/utility/tuple.hpp"
#include "thesauros/utility/value-tag.hpp"

namespace thes {
template<auto tKey, typename TValue>
struct StaticKeyValuePair {
  using Key = decltype(tKey);
  using Value = TValue;
  static constexpr Key key = tKey;

  Value value;
};

template<auto tKey>
struct StaticKey {
  template<typename TValue>
  constexpr StaticKeyValuePair<tKey, TValue> operator=(TValue&& value) const {
    return {std::forward<TValue>(value)};
  }
};

template<auto tKey>
inline constexpr StaticKey<tKey> static_key{};

namespace literals {
template<StaticString tString>
inline constexpr StaticKey<tString> operator""_key() {
  return {};
}
} // namespace literals

template<typename... TPairs>
struct StaticMap;

template<typename... TPairs>
requires(Tuple<typename TPairs::Key...>{TPairs::key...} | star::all_different)
struct StaticMap<TPairs...> {
  using Tuple = ::thes::Tuple<TPairs...>;

  template<auto tKey>
  static constexpr bool contains = [] {
    auto impl = [](auto idx, auto rec) {
      if constexpr (idx == sizeof...(TPairs)) {
        return false;
      } else if constexpr (TupleElement<idx, Tuple>::key == tKey) {
        return true;
      } else {
        return rec(index_tag<idx + 1>, rec);
      }
    };
    return impl(index_tag<0>, impl);
  }();

  template<auto... tKeys>
  static constexpr bool only_keys =
    thes::Tuple{TPairs::key...} |
    star::transform([](auto key) { return thes::Tuple{tKeys...} | star::contains(key); }) |
    star::left_reduce(std::logical_and{}, true);

  explicit constexpr StaticMap(TPairs&&... pairs) : pairs_{std::forward<TPairs>(pairs)...} {}

  template<auto tKey>
  [[nodiscard]] constexpr const auto& get() const {
    return get_impl<tKey>(*this);
  }
  template<auto tKey>
  [[nodiscard]] constexpr auto& get() {
    return get_impl<tKey>(*this);
  }

private:
  template<auto tKey>
  static constexpr auto& get_impl(auto& self) {
    auto impl = [&self](auto idx, auto rec) -> const auto& {
      static_assert(idx < sizeof...(TPairs), "The key is not known!");
      if constexpr (TupleElement<idx, Tuple>::key == tKey) {
        return star::get_at<idx>(self.pairs_).value;
      } else {
        return rec(index_tag<idx + 1>, rec);
      }
    };
    return impl(index_tag<0>, impl);
  }

  Tuple pairs_;
};

template<>
struct StaticMap<> {
  template<auto tKey>
  static constexpr bool contains = false;

  template<auto tKey>
  constexpr auto get() const;
};

template<typename... TPairs>
StaticMap(TPairs&&... pairs) -> StaticMap<TPairs...>;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_MAP_HPP
