// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_SET_ALGORITHMS_HPP
#define INCLUDE_THESAUROS_CONTAINERS_SET_ALGORITHMS_HPP

#include <algorithm>
#include <functional>
#include <ranges>
#include <utility>

namespace thes {
/**
 * `std::erase_if` implemented in terms of `std::ranges::remove_if` and the `erase` member function.
 */
template<typename MutRange, typename Pred>
inline void erase_if(MutRange& r, Pred pred) {
  const auto [first, last] = std::ranges::remove_if(r.begin(), r.end(), pred);
  r.erase(first, last);
}

/**
 * The union of two sorted ranges without duplicates that may not alias each other, with the
 * elements from `r2` that are not in `r1` appended to `r1` in the order they were in in `r2`, which
 * allows `std::ranges::inplace_merge` to be called on `r1` afterwards.
 */
template<typename MutRange, typename OtherRange, typename Cmp = std::ranges::less,
         typename Proj = std::identity>
inline void set_union_unsorted(MutRange& r1, const OtherRange& r2, Cmp cmp = {}, Proj proj = {}) {
  using Size = decltype(std::ranges::size(r1));

  Size i1 = 0;
  const Size size1 = std::ranges::size(r1);
  auto it2 = r2.begin();
  const auto last2 = r2.end();

  // For each element in `r2`, check whether it is present in `r1`; if not, push it to the back of
  // `r1`.
  for (; it2 != last2; ++it2) {
    decltype(auto) e2 = *it2;
    decltype(auto) ri2 = std::invoke(proj, e2);

    while (i1 < size1 && std::invoke(cmp, std::invoke(proj, r1[i1]), ri2)) {
      ++i1;
    }
    if (i1 == size1) {
      break;
    }

    // `proj(r1[i1]) ≥ ri2` has been established; only add if `proj(r1[i1]) > ri2`.
    if (std::invoke(cmp, ri2, std::invoke(proj, r1[i1]))) {
      r1.push_back(std::forward<decltype(e2)>(e2));
    }
  }
  for (; it2 != last2; ++it2) {
    r1.push_back(*it2);
  }
}

/**
 * The stable union of two sorted ranges without duplicates that may not alias each other. `r1` is
 * sorted with respect to `Cmp` after termination.
 */
template<typename MutRange, typename OtherRange, typename Cmp = std::ranges::less,
         typename Proj = std::identity>
inline void set_union(MutRange& r1, const OtherRange& r2, Cmp cmp = {}, Proj proj = {}) {
  const auto s1 = std::ranges::ssize(r1);
  set_union_unsorted(r1, r2, cmp, proj);
  std::ranges::inplace_merge(r1.begin(), r1.begin() + s1, r1.end(), cmp, proj);
}

/**
 * The difference of two sorted ranges without duplicates that may not alias each other; all
 * elements in `r1` that are also contained in `r2` are erased.
 */
template<typename MutRange, typename OtherRange, typename Cmp = std::ranges::less,
         typename Proj = std::identity>
inline void set_difference(MutRange& r1, const OtherRange& r2, Cmp cmp = {}, Proj proj = {}) {
  auto it1 = r1.begin();
  const auto last1 = r1.end();
  auto it2 = r2.begin();
  const auto last2 = r2.end();
  auto last_kept = it1;

  // For each element in `r1`, check whether it is present in `r2`; if not and something has already
  // been removed in `r1`, move it to the end of the range to be kept. Everything past this range at
  // the end is erased.
  for (; it1 != last1; ++it1) {
    decltype(auto) e1 = *it1;
    decltype(auto) ri1 = std::invoke(proj, e1);

    while (it2 != last2 && std::invoke(cmp, std::invoke(proj, *it2), ri1)) {
      ++it2;
    }
    if (it2 == last2) {
      break;
    }
    // `proj(*it2) ≥ ri1` has been established; only keep if `proj(*it2) > ri1`.
    if (std::invoke(cmp, ri1, std::invoke(proj, *it2))) {
      if (last_kept != it1) {
        *last_kept = std::move(*it1);
      }
      ++last_kept;
    }
  }
  if (last_kept == it1) {
    // Nothing to erase.
    return;
  }
  for (; it1 != last1; ++it1, ++last_kept) {
    *last_kept = std::move(*it1);
  }
  r1.erase(last_kept, last1);
}

template<typename It, typename V, typename Cmp = std::ranges::less, typename Proj = std::identity>
inline It find_sorted(It begin, It end, const V& value, Cmp cmp = {}, Proj proj = {}) {
  const It it = std::ranges::lower_bound(begin, end, value, cmp, proj);
  // `lower_bound` ensures that `proj(*it) ≥ value`; only return `it` if `proj(*it) ≤ value`.
  if (it != end && !std::invoke(cmp, value, std::invoke(proj, *it))) {
    return it;
  }
  return end;
}

template<typename It, typename V, typename Cmp = std::ranges::less, typename Proj = std::identity>
inline bool exists_sorted(It begin, It end, const V& value, Cmp cmp = {}, Proj proj = {}) {
  return find_sorted(begin, end, value, cmp, proj) != end;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_SET_ALGORITHMS_HPP
