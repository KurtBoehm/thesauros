#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_ENUMERATE_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_ENUMERATE_HPP

#include <cstddef>
#include <type_traits>

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/static-value.hpp"

namespace thes::star {
template<typename TSize, typename TInner>
struct EnumerateView {
  TInner inner;

  using Inner = std::decay_t<TInner>;
  static constexpr std::size_t size = thes::star::size<Inner>;

  template<std::size_t tIndex>
  constexpr auto get() const {
    return std::make_pair(static_value<TSize, tIndex>, get_at<tIndex>(inner));
  }
};

template<typename TSize>
struct EnumerateGenerator {
  template<typename TRange>
  constexpr EnumerateView<TSize, TRange> operator()(TRange&& range) const {
    return {std::forward<TRange>(range)};
  }
};
template<typename TSize>
struct RangeGeneratorTrait<EnumerateGenerator<TSize>> : public std::true_type {};

template<typename TSize>
inline constexpr EnumerateGenerator<TSize> enumerate{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_ENUMERATE_HPP
