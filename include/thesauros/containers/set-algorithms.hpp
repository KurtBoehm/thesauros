// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_SET_ALGORITHMS_HPP
#define INCLUDE_THESAUROS_CONTAINERS_SET_ALGORITHMS_HPP

#include <algorithm>
#include <cstddef>
#include <functional>

namespace thes {
template<typename MutRange, typename Pred>
inline void erase_if(MutRange& r, Pred pred) {
  r.erase(std::remove_if(r.begin(), r.end(), pred), r.end());
}

template<typename MutRange, typename OtherRange, typename Cmp = std::less<>,
         typename Eq = std::equal_to<>>
inline void set_union_unsorted(MutRange& r1, const OtherRange& r2, Cmp cmp = Cmp{}, Eq eq = Eq{}) {
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
    }
  }
  for (; cur2 != last2; ++cur2) {
    r1.push_back(*cur2);
  }
}

template<typename MutRange, typename OtherRange, typename Cmp = std::less<>,
         typename Eq = std::equal_to<>>
inline void set_union(MutRange& r1, const OtherRange& r2, Cmp cmp = Cmp{}, Eq eq = Eq{}) {
  set_union_unsorted(r1, r2, cmp, eq);
  std::sort(r1.begin(), r1.end(), cmp);
}

template<typename MutRange, typename OtherRange, typename Cmp = std::less<>,
         typename Eq = std::equal_to<>>
void set_difference(MutRange& r1, const OtherRange& r2, Cmp cmp = Cmp{}, Eq eq = Eq{}) {
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

template<typename It, typename V, typename Cmp = std::less<>, typename Eq = std::equal_to<>>
inline It find_sorted(It begin, It end, const V& value, Cmp cmp = Cmp{}, Eq eq = Eq{}) {
  const It it{std::lower_bound(begin, end, value, cmp)};
  if (it != end && eq(*it, value)) {
    return it;
  }
  return end;
}

template<typename It, typename V, typename Cmp = std::less<>, typename Eq = std::equal_to<>>
inline bool exists_sorted(It begin, It end, const V& value, Cmp cmp = Cmp{}, Eq eq = Eq{}) {
  const It it{std::lower_bound(begin, end, value, cmp)};
  return it != end && eq(*it, value);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_SET_ALGORITHMS_HPP
