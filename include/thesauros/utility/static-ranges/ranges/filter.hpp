#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_FILTER_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_FILTER_HPP

#include <cstddef>
#include <type_traits>

#include "thesauros/utility/static-ranges/definitions/concepts.hpp"
#include "thesauros/utility/static-ranges/definitions/size.hpp"
#include "thesauros/utility/value-sequence.hpp"

namespace thes::star {
template<typename TInner, typename TIndices>
struct FilterView {
  TInner inner;

  static constexpr std::size_t size = TIndices::size;

  template<std::size_t tIndex>
  constexpr decltype(auto) get() const {
    return get_at<TIndices::template get_at<tIndex>>(inner);
  }
};

template<typename TIndices>
struct FilterGenerator {
  template<typename TRange>
  constexpr FilterView<TRange, TIndices> operator()(TRange&& range) const {
    return {std::forward<TRange>(range)};
  }
};
template<typename TIndices>
struct RangeGeneratorTrait<FilterGenerator<TIndices>> : public std::true_type {};

template<typename TIndices>
struct AllExceptGenerator {
  template<typename TRange>
  constexpr auto operator()(TRange&& range) const {
    using AllIdxs = MakeIntegerSequence<std::size_t, 0, size<TRange>>;
    using Return = FilterView<TRange, typename AllIdxs::template ExceptSequence<TIndices>>;
    return Return{std::forward<TRange>(range)};
  }
};
template<typename TIndices>
struct RangeGeneratorTrait<AllExceptGenerator<TIndices>> : public std::true_type {};

template<std::size_t... tIdxs>
inline constexpr FilterGenerator<ValueSequence<std::size_t, tIdxs...>> only_idxs{};
template<typename TIdxs>
inline constexpr FilterGenerator<TIdxs> only_idxseq{};
template<std::size_t... tIdxs>
inline constexpr AllExceptGenerator<ValueSequence<std::size_t, tIdxs...>> all_except_idxs{};
template<typename TIdxs>
inline constexpr AllExceptGenerator<TIdxs> all_except_idxseq{};
template<std::size_t tBegin, std::size_t tEnd>
inline constexpr FilterGenerator<MakeAutoIntegerSequence<tBegin, tEnd>> sub_range{};
} // namespace thes::star

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_RANGES_RANGES_FILTER_HPP
