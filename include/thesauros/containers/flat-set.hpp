// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_FLAT_SET_HPP
#define INCLUDE_THESAUROS_CONTAINERS_FLAT_SET_HPP

#include <algorithm>
#include <cstddef>
#include <functional>

#include "thesauros/containers/array/dynamic.hpp"
#include "thesauros/containers/set-algorithms.hpp"

namespace thes {
/** A container adaptor that models a sorted set based on an underlying random-access container. */
template<typename V, typename Cmp = std::ranges::less, typename C = DynamicArray<V>>
struct FlatSet {
  using Value = V;
  using Container = C;

  using value_type = Value;
  using size_type = Container::size_type;
  using const_iterator = Container::const_iterator;

  FlatSet() = default;
  /** Constructs a flat set from an underlying container that is assumed to be sorted already. */
  explicit FlatSet(C&& container) : data_{std::forward<C>(container)} {}

  [[nodiscard]] const_iterator begin() const {
    return data_.begin();
  }
  [[nodiscard]] const_iterator end() const {
    return data_.end();
  }

  [[nodiscard]] const_iterator cbegin() const {
    return begin();
  }
  [[nodiscard]] const_iterator cend() const {
    return end();
  }

  [[nodiscard]] size_type size() const {
    return data_.size();
  }
  [[nodiscard]] bool empty() const {
    return data_.empty();
  }

  [[nodiscard]] const V& front() const {
    return data_.front();
  }
  void pop_front() {
    data_.erase(data_.begin());
  }

  [[nodiscard]] const_iterator lower_bound(const auto& value) const {
    return lower_bound_impl(value);
  }

  [[nodiscard]] bool contains(const auto& value) const {
    const auto it = lower_bound_impl(value);
    // `lower_bound` ensures that `*it ≥ value`; `*it ≤ value` remains to be checked.
    return it != end() && !compare_(value, *it);
  }

  [[nodiscard]] const_iterator find(const auto& value) const {
    const auto it = lower_bound_impl(value);
    // `lower_bound` ensures that `*it ≥ value`; `*it ≤ value` remains to be checked.
    if (it != end() && !compare_(value, *it)) {
      return it;
    }
    return end();
  }

  void insert(const V& value) {
    auto it = lower_bound_impl(value);
    // `lower_bound` ensures that `*it ≥ value`; `*it ≤ value` remains to be checked.
    if (it != end() && !compare_(value, *it)) {
      return;
    }
    data_.insert(it, value);
  }
  void insert(V&& value) {
    auto it = lower_bound_impl(value);
    // `lower_bound` ensures that `*it ≥ value`; `*it ≤ value` remains to be checked.
    if (it != end() && !compare_(value, *it)) {
      return;
    }
    data_.insert(it, std::forward<V>(value));
  }

  bool erase(const auto& value) {
    const auto it = lower_bound_impl(value);
    // `lower_bound` ensures that `*it ≥ value`; `*it ≤ value` remains to be checked.
    if (it != end() && !compare_(value, *it)) {
      data_.erase(it);
      return true;
    }
    return false;
  }

  template<typename Other>
  void set_difference(const Other& other) {
    thes::set_difference(data_, other, compare_);
  }

  template<typename Pred>
  void erase_if(Pred pred) {
    thes::erase_if(data_, pred);
  }

  template<typename Other>
  void set_union(const Other& other) {
    thes::set_union(data_, other, compare_);
  }

  void clear() {
    data_.clear();
  }

  [[nodiscard]] const Container& container() const {
    return data_;
  }

private:
  [[nodiscard]] const_iterator lower_bound_impl(const auto& value) const {
    return std::lower_bound(data_.begin(), data_.end(), value, compare_);
  }
  [[nodiscard]] auto lower_bound_impl(const auto& value) {
    return std::lower_bound(data_.begin(), data_.end(), value, compare_);
  }

  Container data_{};
  [[no_unique_address]] Cmp compare_{};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_FLAT_SET_HPP
