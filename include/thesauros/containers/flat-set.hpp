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
template<typename V, typename Cmp = std::less<V>, typename Eq = std::equal_to<V>,
         typename C = DynamicArray<V>>
struct FlatSet {
  using Value = V;
  using Container = C;

  using value_type = Value;
  using const_iterator = Container::const_iterator;

  FlatSet() = default;

  const_iterator begin() const {
    return data_.begin();
  }
  const_iterator end() const {
    return data_.end();
  }

  const_iterator cbegin() const {
    return begin();
  }
  const_iterator cend() const {
    return end();
  }

  [[nodiscard]] std::size_t size() const {
    return data_.size();
  }
  [[nodiscard]] bool empty() const {
    return data_.empty();
  }

  const V& front() const {
    return data_.front();
  }
  void pop_front() {
    data_.erase(data_.begin());
  }

  const_iterator lower_bound(const auto& value) const {
    return std::lower_bound(data_.begin(), data_.end(), value, compare_);
  }

  bool contains(const auto& value) const {
    const auto it{lower_bound(value)};
    return it != end() && equal_(*it, value);
  }

  const_iterator find(const auto& value) const {
    const auto it{lower_bound(value)};
    if (it != end() && equal_(*it, value)) {
      return it;
    }
    return end();
  }

  void insert(const V& value) {
    auto it = lower_bound(value);
    if (it != end() && equal_(*it, value)) {
      return;
    }
    data_.insert(it, value);
  }

  bool erase(const auto& value) {
    const auto it{lower_bound(value)};
    if (it != end() && equal_(*it, value)) {
      data_.erase(it);
      return true;
    }
    return false;
  }

  template<typename Other>
  void set_difference(const Other& other) {
    thes::set_difference(data_, other);
  }

  template<typename Pred>
  void erase_if(Pred pred) {
    thes::erase_if(data_, pred);
  }

  template<typename Other>
  void set_union(const Other& other) {
    thes::set_union(data_, other, compare_, equal_);
  }

  void clear() {
    data_.clear();
  }

private:
  auto lower_bound(const auto& value) {
    return std::lower_bound(data_.begin(), data_.end(), value, compare_);
  }

  Container data_{};
  [[no_unique_address]] Cmp compare_{};
  [[no_unique_address]] Eq equal_{};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_FLAT_SET_HPP
