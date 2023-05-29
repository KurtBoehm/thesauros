#ifndef INCLUDE_THESAUROS_CONTAINERS_SET_ALGORITHMS_HPP
#define INCLUDE_THESAUROS_CONTAINERS_SET_ALGORITHMS_HPP

#include <algorithm>
#include <cstddef>
#include <functional>

namespace thes {
template<typename TMutRange, typename TPred>
inline void erase_if(TMutRange& r, TPred pred) {
  r.erase(std::remove_if(r.begin(), r.end(), pred), r.end());
}

template<typename TMutRange, typename TOtherRange, typename TCompare = std::less<>,
         typename TEqual = std::equal_to<>>
inline void set_union_unsorted(TMutRange& r1, const TOtherRange& r2, TCompare cmp = TCompare{},
                               TEqual eq = TEqual{}) {
  std::size_t i1{0};
  const std::size_t size1{r1.size()};
  auto cur2{r2.begin()};
  const auto last2{r2.end()};

  for (; cur2 != last2; ++cur2) {
    while (i1 < size1 && cmp(r1[i1], *cur2)) {
      ++i1;
    }
    if (i1 == size1) {
      break;
    }

    if (!eq(r1[i1], *cur2)) {
      r1.push_back(*cur2);
      auto f1{r1.begin()};
    }
  }
  for (; cur2 != last2; ++cur2) {
    r1.push_back(*cur2);
  }
}

template<typename TMutRange, typename TOtherRange, typename TCompare = std::less<>,
         typename TEqual = std::equal_to<>>
inline void set_union(TMutRange& r1, const TOtherRange& r2, TCompare cmp = TCompare{},
                      TEqual eq = TEqual{}) {
  set_union_unsorted(r1, r2, cmp, eq);
  std::sort(r1.begin(), r1.end(), cmp);
}

template<typename TMutRange, typename TOtherRange, typename TCompare = std::less<>,
         typename TEqual = std::equal_to<>>
void set_difference(TMutRange& r1, const TOtherRange& r2, TCompare cmp = TCompare{},
                    TEqual eq = TEqual{}) {
  auto first1{r1.begin()};
  const auto last1{r1.end()};
  auto first2{r2.begin()};
  const auto last2{r2.end()};
  auto last_valid{first1};

  for (; first1 != last1; ++first1) {
    while (first2 != last2 && cmp(*first2, *first1)) {
      ++first2;
    }
    if (first2 == last2) {
      break;
    }
    if (!eq(*first1, *first2)) {
      std::iter_swap(last_valid++, first1);
    }
  }
  while (first1 != last1) {
    std::iter_swap(first1++, last_valid++);
  }
  r1.erase(last_valid, last1);
}

template<typename TIt, typename TValue, typename TCompare = std::less<>,
         typename TEqual = std::equal_to<>>
inline TIt find_sorted(TIt begin, TIt end, const TValue& value, TCompare cmp = TCompare{},
                       TEqual eq = TEqual{}) {
  const TIt it{std::lower_bound(begin, end, value, cmp)};
  if (it != end && eq(*it, value)) {
    return it;
  }
  return end;
}

template<typename TIt, typename TValue, typename TCompare = std::less<>,
         typename TEqual = std::equal_to<>>
inline bool exists_sorted(TIt begin, TIt end, const TValue& value, TCompare cmp = TCompare{},
                          TEqual eq = TEqual{}) {
  const TIt it{std::lower_bound(begin, end, value, cmp)};
  return it != end && eq(*it, value);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_SET_ALGORITHMS_HPP
