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

namespace thes::star {
template<typename... TRanges>
struct Join {
  using Tuple = std::tuple<TRanges...>;
  Tuple ranges;

  static constexpr std::size_t size = (... + star::size<TRanges>);

  template<std::size_t tIndex>
  requires(tIndex < size)
  constexpr auto get() const {
    constexpr auto pair = []() {
      std::size_t sum = 0;
      std::optional<std::pair<std::size_t, std::size_t>> out{};
      star::iota<0, sizeof...(TRanges)> | star::for_each([&](auto idx) {
        constexpr std::size_t idx_size = star::size<std::tuple_element_t<idx, Tuple>>;
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
    return get_at<pair.second>(std::get<pair.first>(ranges));
  }
};

template<typename... TRanges>
requires(sizeof...(TRanges) > 0)
inline constexpr Join<TRanges...> joined(TRanges&&... ranges) {
  return {std::make_tuple(std::forward<TRanges>(ranges)...)};
}
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_JOIN_HPP
