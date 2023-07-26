#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_FILTER_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_FILTER_HPP

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/static-ranges/ranges/iota.hpp"
#include "thesauros/utility/static-ranges/ranges/transform.hpp"
#include "thesauros/utility/static-ranges/sinks/for-each.hpp"
#include "thesauros/utility/static-ranges/sinks/to-array.hpp"

namespace thes::star {
template<typename TInner, auto tIdxRange>
struct FilterView {
  TInner inner;

  using IdxRange = std::decay_t<decltype(tIdxRange)>;
  static constexpr std::size_t size = star::size<IdxRange>;

  template<std::size_t tIndex>
  constexpr decltype(auto) get() const {
    return get_at<get_at<tIndex>(tIdxRange)>(inner);
  }
};

template<auto tIdxRange>
struct FilterGenerator : public RangeGeneratorBase {
  template<typename TRange>
  constexpr FilterView<TRange, tIdxRange> operator()(TRange&& range) const {
    return {std::forward<TRange>(range)};
  }
};

template<auto tIdxRange>
struct AllExceptGenerator : public RangeGeneratorBase {
  template<typename TRange>
  constexpr auto operator()(TRange&& range) const {
    constexpr std::size_t range_size = star::size<TRange>;

    constexpr auto pair = [&] {
      std::array<std::size_t, range_size> buffer{};
      std::size_t count = 0;

      star::for_each([&](auto i) {
        bool contains = false;
        star::for_each([&](auto j) { contains = contains || (i == j); })(tIdxRange);
        if (!contains) {
          buffer[count] = i;
          ++count;
        }
      })(star::iota<0, range_size>);

      return std::make_pair(buffer, count);
    }();
    constexpr auto idxs = star::to_array(
      star::index_transform<pair.second>([&](auto idx) { return std::get<idx>(pair.first); }));

    return FilterView<TRange, idxs>{std::forward<TRange>(range)};
  }
};

template<std::size_t... tIdxs>
inline constexpr FilterGenerator<std::array<std::size_t, sizeof...(tIdxs)>{tIdxs...}> only_idxs{};
template<auto tIdxRange>
inline constexpr FilterGenerator<tIdxRange> only_range{};
template<std::size_t... tIdxs>
inline constexpr AllExceptGenerator<std::array<std::size_t, sizeof...(tIdxs)>{tIdxs...}>
  all_except_idxs{};
template<auto tIdxRange>
inline constexpr AllExceptGenerator<tIdxRange> all_except_range{};
template<std::size_t tBegin, std::size_t tEnd>
inline constexpr FilterGenerator<star::iota<tBegin, tEnd>> sub_range{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_FILTER_HPP
