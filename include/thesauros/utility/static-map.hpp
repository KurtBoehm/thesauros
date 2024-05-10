#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_MAP_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_MAP_HPP

#include <functional>
#include <type_traits>
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
template<auto tKey, auto tValue>
inline constexpr StaticKeyValuePair<tKey, decltype(tValue)> static_kv{tValue};

namespace literals {
template<StaticString tString>
inline constexpr StaticKey<tString> operator""_key() {
  return {};
}
} // namespace literals

template<typename... TPairs>
struct StaticMap;

template<typename... TPairs>
requires(Tuple<typename std::decay_t<TPairs>::Key...>{std::decay_t<TPairs>::key...} |
         star::all_different)
struct StaticMap<TPairs...> {
  using Tuple = ::thes::Tuple<TPairs...>;
  using DecayedTuple = ::thes::Tuple<std::decay_t<TPairs>...>;

  static constexpr bool contains(AnyValueTag auto key) {
    auto impl = [key](auto idx, auto rec) {
      if constexpr (idx == sizeof...(TPairs)) {
        return false;
      } else if constexpr (TupleElement<idx, DecayedTuple>::key == key.value) {
        return true;
      } else {
        return rec(index_tag<idx + 1>, rec);
      }
    };
    return impl(index_tag<0>, impl);
  }

  template<auto... tKeys>
  static constexpr bool only_keys =
    thes::Tuple{TPairs::key...} |
    star::transform([](auto key) { return thes::Tuple{tKeys...} | star::contains(key); }) |
    star::left_reduce(std::logical_and{}, true);

  explicit constexpr StaticMap(TPairs&&... pairs) : _pairs{std::forward<TPairs>(pairs)...} {}

  [[nodiscard]] constexpr const auto& get(AnyValueTag auto key) const {
    return get_impl<key.value>(*this);
  }
  [[nodiscard]] constexpr auto& get(AnyValueTag auto key) {
    return get_impl<key.value>(*this);
  }

  [[nodiscard]] constexpr const auto& get(AnyValueTag auto key, const auto& def) const {
    return get_impl<key.value>(*this, def);
  }
  [[nodiscard]] constexpr auto& get(AnyValueTag auto key, auto& def) {
    return get_impl<key.value>(*this, def);
  }

  // _pairs must be public for StaticMap to be a structural type!
  Tuple _pairs;

private:
  template<auto tKey>
  static constexpr auto& get_impl(auto& self) {
    auto impl = [&self](auto idx, auto rec) -> const auto& {
      static_assert(idx < sizeof...(TPairs), "The key is not known!");
      if constexpr (TupleElement<idx, DecayedTuple>::key == tKey) {
        return star::get_at<idx>(self._pairs).value;
      } else {
        return rec(index_tag<idx + 1>, rec);
      }
    };
    return impl(index_tag<0>, impl);
  }

  template<auto tKey>
  static constexpr auto& get_impl(auto& self, auto& def) {
    auto impl = [&](auto idx, auto rec) -> const auto& {
      if constexpr (idx == sizeof...(TPairs)) {
        return def;
      } else if constexpr (TupleElement<idx, DecayedTuple>::key == tKey) {
        return star::get_at<idx>(self._pairs).value;
      } else {
        return rec(index_tag<idx + 1>, rec);
      }
    };
    return impl(index_tag<0>, impl);
  }
};

template<>
struct StaticMap<> {
  static constexpr bool contains(AnyValueTag auto /*key*/) {
    return false;
  }
  template<auto... tKeys>
  static constexpr bool only_keys = true;

  constexpr auto get(AnyValueTag auto key) const;

  [[nodiscard]] constexpr const auto& get(AnyValueTag auto /*key*/, const auto& def) const {
    return def;
  }
  [[nodiscard]] constexpr auto& get(AnyValueTag auto /*key*/, auto& def) {
    return def;
  }
};

template<typename... TPairs>
StaticMap(TPairs&&... pairs) -> StaticMap<TPairs...>;

template<auto... tPairs>
inline constexpr auto static_map_tag =
  thes::auto_tag<StaticMap((std::decay_t<decltype(tPairs)>(tPairs))...)>;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_MAP_HPP
