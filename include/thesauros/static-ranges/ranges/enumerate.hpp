#ifndef INCLUDE_THESAUROS_STATIC_RANGES_RANGES_ENUMERATE_HPP
#define INCLUDE_THESAUROS_STATIC_RANGES_RANGES_ENUMERATE_HPP

#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/utility/inlining.hpp"
#include "thesauros/static-ranges/definitions/concepts.hpp"
#include "thesauros/static-ranges/definitions/size.hpp"
#include "thesauros/utility/value-tag.hpp"

namespace thes::star {
template<typename TSize, typename TInner>
struct EnumerateView {
  TInner inner;

  using Inner = std::decay_t<TInner>;
  static constexpr std::size_t size = thes::star::size<Inner>;

  template<std::size_t tIndex>
  THES_ALWAYS_INLINE constexpr std::pair<ValueTag<TSize, tIndex>, decltype(get_at<tIndex>(inner))>
  get() const {
    return {value_tag<TSize, tIndex>, get_at<tIndex>(inner)};
  }
};

template<typename TSize>
struct EnumerateGenerator : public RangeGeneratorBase {
  template<typename TRange>
  THES_ALWAYS_INLINE constexpr EnumerateView<TSize, TRange> operator()(TRange&& range) const {
    return {std::forward<TRange>(range)};
  }
};

template<typename TSize = std::size_t>
inline constexpr EnumerateGenerator<TSize> enumerate{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_STATIC_RANGES_RANGES_ENUMERATE_HPP
