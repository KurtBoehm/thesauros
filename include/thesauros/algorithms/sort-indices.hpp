#ifndef INCLUDE_THESAUROS_ALGORITHMS_SORT_INDICES_HPP
#define INCLUDE_THESAUROS_ALGORITHMS_SORT_INDICES_HPP

namespace thes {
template<typename TRandomIt, typename TSize, typename TCompare, typename TSwap>
constexpr void sort_indices(TRandomIt&& first, const TSize size, TCompare comp, TSwap swap) {
  for (TSize i = 1; i < size; ++i) {
    for (TSize j = i; j > 0 && comp(first[j], first[j - 1]); --j) {
      swap(j - 1, j);
    }
  }
}
} // namespace thes

#endif // INCLUDE_THESAUROS_ALGORITHMS_SORT_INDICES_HPP
