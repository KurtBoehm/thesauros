#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_JOIN_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_JOIN_HPP

#include <cstddef>
#include <cstdlib>
#include <optional>
#include <tuple>
#include <utility>

#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/static-ranges/ranges/iota.hpp"
#include "thesauros/utility/static-ranges/sinks/for-each.hpp"
#include "thesauros/utility/tuple.hpp"

namespace thes::star {
template<typename... TRanges>
struct Join {
  using Tuple = Tuple<TRanges...>;
  Tuple ranges;

  explicit constexpr Join(TRanges&&... rngs) : ranges{std::forward<TRanges>(rngs)...} {}

  static constexpr std::size_t size = (... + star::size<TRanges>);

  template<std::size_t tIndex>
  requires(tIndex < size)
  constexpr auto get() const {
    constexpr auto pair = []() {
      std::size_t sum = 0;
      std::optional<std::pair<std::size_t, std::size_t>> out{};
      star::iota<0, sizeof...(TRanges)> | star::for_each([&](auto idx) {
        constexpr std::size_t idx_size = star::size<thes::TupleElement<idx, Tuple>>;
        if (sum <= tIndex && tIndex < sum + idx_size) {
          if (out.has_value()) {
            std::abort();
          }
          out = std::make_pair(idx.value, tIndex - sum);
        }
        sum += idx_size;
      });
      return *out;
    }();
    return get_at<pair.second>(get_at<pair.first>(ranges));
  }
};
template<typename... TRanges>
Join(TRanges&&...) -> Join<TRanges...>;

template<typename... TRanges>
requires(sizeof...(TRanges) > 0)
inline constexpr auto joined(TRanges&&... ranges) {
  return Join{std::forward<TRanges>(ranges)...};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_JOIN_HPP
