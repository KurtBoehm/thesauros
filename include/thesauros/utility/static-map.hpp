#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_MAP_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_MAP_HPP

#include <cstddef>
#include <utility>

#include "thesauros/utility/static-ranges/sinks/all-different.hpp"
#include "thesauros/utility/tuple.hpp"
#include "thesauros/utility/type-sequence.hpp"
#include "thesauros/utility/value-tag.hpp"

namespace thes {
template<auto tKey, typename TValue>
struct KeyValuePair {
  using Key = decltype(tKey);
  using Value = TValue;
  static constexpr Key key = tKey;

  Value value;
};

template<auto tKey>
struct StaticKey {
  template<typename TValue>
  constexpr KeyValuePair<tKey, TValue> operator=(TValue&& value) const {
    return {std::forward<TValue>(value)};
  }
};

template<auto tKey>
inline constexpr StaticKey<tKey> static_key{};

template<typename... TPairs>
struct StaticMap;

template<typename... TPairs>
requires(TypeSeq<typename TPairs::Key...>::is_unique &&
         Tuple<typename TPairs::Key...>{TPairs::key...} | star::all_different)
struct StaticMap<TPairs...> {
  using Tuple = ::thes::Tuple<TPairs...>;
  using Key = TypeSeq<typename TPairs::Key...>::Unique;

  template<Key tKey>
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

  explicit constexpr StaticMap(TPairs&&... pairs) : pairs_{std::forward<TPairs>(pairs)...} {}

  template<Key tKey>
  [[nodiscard]] constexpr const auto& get() const {
    return get_impl<tKey>(*this);
  }
  template<Key tKey>
  [[nodiscard]] constexpr auto& get() {
    return get_impl<tKey>(*this);
  }

private:
  template<Key tKey>
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
