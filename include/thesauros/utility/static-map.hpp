#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_MAP_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_MAP_HPP

#include <utility>

#include "thesauros/utility/static-value.hpp"
#include "thesauros/utility/type-sequence.hpp"
#include "thesauros/utility/value-sequence.hpp"

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
requires(TypeSeq<typename TPairs::Key...>::is_unique && AutoSequence<TPairs::key...>::all_different)
struct StaticMap<TPairs...> {
  using Tuple = std::tuple<TPairs...>;
  using Key = typename TypeSeq<typename TPairs::Key...>::Unique;

  template<Key tKey>
  static constexpr bool contains = [] {
    auto impl = []<std::size_t tIdx>(StaticAuto<tIdx> /*tag*/, auto rec) {
      if constexpr (tIdx == sizeof...(TPairs)) {
        return false;
      } else if constexpr (std::tuple_element_t<tIdx, Tuple>::key == tKey) {
        return true;
      } else {
        return rec(static_auto<tIdx + 1>, rec);
      }
    };
    return impl(static_value<std::size_t, 0>, impl);
  }();

  constexpr explicit StaticMap(TPairs&&... pairs) : pairs_{std::forward<TPairs>(pairs)...} {}

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
    auto impl = [&self]<std::size_t tIdx>(StaticAuto<tIdx> /*tag*/, auto rec) -> const auto& {
      static_assert(tIdx < sizeof...(TPairs), "The key is not known!");
      if constexpr (std::tuple_element_t<tIdx, Tuple>::key == tKey) {
        return std::get<tIdx>(self.pairs_).value;
      } else {
        return rec(static_auto<tIdx + 1>, rec);
      }
    };
    return impl(static_value<std::size_t, 0>, impl);
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
