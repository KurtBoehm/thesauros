#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_REVERSED_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_REVERSED_HPP

#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/utility/inlining.hpp"
#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/get-at.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"

namespace thes::star {
template<typename TInner>
struct ReversedView {
  TInner inner;

  using Inner = std::decay_t<TInner>;
  static constexpr std::size_t size = thes::star::size<Inner>;

  template<std::size_t tIndex>
  THES_ALWAYS_INLINE constexpr auto get() const {
    return get_at<size - tIndex - 1>(inner);
  }
};

struct ReversedGenerator : public RangeGeneratorBase {
  template<typename TRange>
  THES_ALWAYS_INLINE constexpr ReversedView<TRange> operator()(TRange&& range) const {
    return {std::forward<TRange>(range)};
  }
};

inline constexpr ReversedGenerator reversed{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_REVERSED_HPP
