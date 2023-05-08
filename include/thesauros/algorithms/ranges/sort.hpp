#ifndef INCLUDE_THESAUROS_ALGORITHMS_RANGES_SORT_HPP
#define INCLUDE_THESAUROS_ALGORITHMS_RANGES_SORT_HPP

namespace thes {
template<typename TSize, typename TCompare, typename TSwap>
constexpr void sort_indices(const TSize size, TCompare comp, TSwap swap) {
  for (TSize i = 1; i < size; ++i) {
    for (TSize j = i; j > 0 && comp(j, j - 1); --j) {
      swap(j - 1, j);
    }
  }
}
} // namespace thes

#endif // INCLUDE_THESAUROS_ALGORITHMS_RANGES_SORT_HPP
