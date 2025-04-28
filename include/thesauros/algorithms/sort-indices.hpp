#ifndef INCLUDE_THESAUROS_ALGORITHMS_SORT_INDICES_HPP
#define INCLUDE_THESAUROS_ALGORITHMS_SORT_INDICES_HPP

namespace thes {
template<typename TSize>
constexpr void sort_indices(auto first, const TSize size, auto comp, auto swap) {
  for (TSize i = 1; i < size; ++i) {
    for (TSize j = i; j > 0 && comp(first[j], first[j - 1]); --j) {
      swap(j - 1, j);
    }
  }
}
} // namespace thes

#endif // INCLUDE_THESAUROS_ALGORITHMS_SORT_INDICES_HPP
