// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_MAP_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_MAP_HPP

#include <functional>
#include <type_traits>
#include <utility>

#include "thesauros/static-ranges/definitions/get-at.hpp"
#include "thesauros/static-ranges/piping.hpp" // IWYU pragma: keep
#include "thesauros/static-ranges/sinks/all-different.hpp"
#include "thesauros/static-ranges/sinks/contains.hpp"
#include "thesauros/static-ranges/sinks/reduce.hpp"
#include "thesauros/static-ranges/views/transform.hpp"
#include "thesauros/string/static-string.hpp"
#include "thesauros/types/tuple.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes {
template<auto K, typename V>
struct StaticKeyValuePair {
  using Key = decltype(K);
  using Value = V;
  static constexpr Key key = K;

  Value value;
};

template<auto K>
struct StaticKey {
  template<typename V>
  constexpr StaticKeyValuePair<K, V> operator=(V&& value) const {
    return {std::forward<V>(value)};
  }
};

template<auto K>
inline constexpr StaticKey<K> static_key{};
template<auto K, auto V>
inline constexpr StaticKeyValuePair<K, decltype(V)> static_kv{V};

inline namespace literals {
inline namespace static_map_literals {
template<StaticString String>
constexpr StaticKey<String> operator""_key() {
  return {};
}
} // namespace static_map_literals
} // namespace literals

template<typename... Pairs>
struct StaticMap;

template<typename... Pairs>
requires(Tuple<typename std::decay_t<Pairs>::Key...>{std::decay_t<Pairs>::key...} |
         star::all_different)
struct StaticMap<Pairs...> {
  using Tuple = ::thes::Tuple<Pairs...>;
  using DecayedTuple = ::thes::Tuple<std::decay_t<Pairs>...>;

  static constexpr bool contains(AnyValueTag auto key) {
    auto impl = [key](auto idx, auto rec) {
      if constexpr (idx == sizeof...(Pairs)) {
        return false;
      } else if constexpr (TupleElement<idx, DecayedTuple>::key == key.value) {
        return true;
      } else {
        return rec(index_tag<idx + 1>, rec);
      }
    };
    return impl(index_tag<0>, impl);
  }

  template<auto... K>
  static constexpr bool only_keys =
    thes::Tuple{Pairs::key...} |
    star::transform([](auto key) { return thes::Tuple{K...} | star::contains(key); }) |
    star::left_reduce(std::logical_and{}, true);

  explicit constexpr StaticMap(Pairs&&... pairs) : _pairs{std::forward<Pairs>(pairs)...} {}

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
  template<auto K>
  static constexpr auto& get_impl(auto& self) {
    auto impl = [&self](auto idx, auto rec) -> const auto& {
      static_assert(idx < sizeof...(Pairs), "The key is not known!");
      if constexpr (TupleElement<idx, DecayedTuple>::key == K) {
        return star::get_at<idx>(self._pairs).value;
      } else {
        return rec(index_tag<idx + 1>, rec);
      }
    };
    return impl(index_tag<0>, impl);
  }

  template<auto K>
  static constexpr auto& get_impl(auto& self, auto& def) {
    auto impl = [&](auto idx, auto rec) -> const auto& {
      if constexpr (idx == sizeof...(Pairs)) {
        return def;
      } else if constexpr (TupleElement<idx, DecayedTuple>::key == K) {
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
  template<auto... K>
  static constexpr bool only_keys = true;

  constexpr auto get(AnyValueTag auto key) const;

  [[nodiscard]] constexpr const auto& get(AnyValueTag auto /*key*/, const auto& def) const {
    return def;
  }
  [[nodiscard]] constexpr auto& get(AnyValueTag auto /*key*/, auto& def) {
    return def;
  }
};

template<typename... Pairs>
StaticMap(Pairs&&... pairs) -> StaticMap<Pairs...>;

template<auto... Pairs>
inline constexpr auto static_map_tag =
  auto_tag<StaticMap((std::decay_t<decltype(Pairs)>(Pairs))...)>;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_MAP_HPP
