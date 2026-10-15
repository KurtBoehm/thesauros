// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONTAINERS_FLAT_MAP_HPP
#define INCLUDE_THESAUROS_CONTAINERS_FLAT_MAP_HPP

#include <algorithm>
#include <cstddef>
#include <functional>
#include <utility>

#include "thesauros/containers/array/dynamic.hpp"
#include "thesauros/containers/set-algorithms.hpp"

namespace thes {
template<typename K, typename V, typename KCmp = std::less<K>,
         typename C = DynamicArray<std::pair<K, V>>>
struct FlatMap {
  using Key = K;
  using Mapped = V;
  using KeyCompare = KCmp;

  using Value = std::pair<Key, Mapped>;
  using Container = C;

  using value_type = Value;
  using iterator = Container::iterator;
  using const_iterator = Container::const_iterator;

  FlatMap() = default;

  [[nodiscard]] iterator begin() {
    return data_.begin();
  }
  [[nodiscard]] const_iterator begin() const {
    return data_.begin();
  }
  [[nodiscard]] iterator end() {
    return data_.end();
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

  [[nodiscard]] std::size_t size() const {
    return data_.size();
  }
  [[nodiscard]] bool empty() const {
    return data_.empty();
  }

  [[nodiscard]] const Value& front() const {
    return data_.front();
  }
  [[nodiscard]] Value& front() {
    return data_.front();
  }
  void pop_front() {
    data_.erase(data_.begin());
  }

  [[nodiscard]] iterator lower_bound(const auto& key) {
    return std::ranges::lower_bound(data_.begin(), data_.end(), key, compare_,
                                    [](const auto& p) -> const auto& { return p.first; });
  }
  [[nodiscard]] const_iterator lower_bound(const auto& key) const {
    return std::ranges::lower_bound(data_.begin(), data_.end(), key, compare_,
                                    [](const auto& p) -> const auto& { return p.first; });
  }

  [[nodiscard]] bool contains(const auto& key) const {
    const auto it = lower_bound(key);
    // `lower_bound` ensures that `it->first ≥ key`; `it->first ≤ key` remains to be checked.
    return it != end() && !compare_(key, it->first);
  }

  [[nodiscard]] iterator find(const auto& key) {
    const auto it = lower_bound(key);
    // `lower_bound` ensures that `it->first ≥ key`; `it->first ≤ key` remains to be checked.
    if (it != end() && !compare_(key, it->first)) {
      return it;
    }
    return end();
  }
  [[nodiscard]] const_iterator find(const auto& key) const {
    const auto it = lower_bound(key);
    // `lower_bound` ensures that `it->first ≥ key`; `it->first ≤ key` remains to be checked.
    if (it != end() && !compare_(key, it->first)) {
      return it;
    }
    return end();
  }

  bool insert(const Key& key, const Mapped& value) {
    const auto it = lower_bound(key);
    // `lower_bound` ensures that `it->first ≥ key`; `it->first ≤ key` remains to be checked.
    if (it != end() && !compare_(key, it->first)) {
      return false;
    }
    data_.insert(it, Value{key, value});
    return true;
  }

  Mapped& get_or_insert(const Key& key, Mapped&& value) {
    const auto it = lower_bound(key);
    // `lower_bound` ensures that `it->first ≥ key`; `it->first ≤ key` remains to be checked.
    if (it != end() && !compare_(key, it->first)) {
      return it->second;
    }
    return data_.insert(it, Value{key, std::forward<Mapped>(value)})->second;
  }
  template<typename Trans, typename Create>
  void transform_or_create(const Key& key, Trans&& transform, Create&& create) {
    const auto it = lower_bound(key);
    // `lower_bound` ensures that `it->first ≥ key`; `it->first ≤ key` remains to be checked.
    if (it != end() && !compare_(key, it->first)) {
      std::forward<Trans>(transform)(it->second);
      return;
    }
    data_.insert(it, Value{key, std::forward<Create>(create)()});
  }

  bool erase(const auto& key) {
    const auto it = lower_bound(key);
    // `lower_bound` ensures that `it->first ≥ key`; `it->first ≤ key` remains to be checked.
    if (it != end() && !compare_(key, it->first)) {
      data_.erase(it);
      return true;
    }
    return false;
  }

  [[nodiscard]] Mapped& at(const auto& key) {
    return find(key)->second;
  }
  [[nodiscard]] const Mapped& at(const auto& key) const {
    return find(key)->second;
  }

  template<typename Other>
  void set_difference(const Other& other) {
    thes::set_difference(data_, other, compare_,
                         [](const auto& p) -> const auto& { return p.first; });
  }

  template<typename Pred>
  void erase_if(Pred pred) {
    thes::erase_if(data_, pred);
  }

  template<typename Other>
  void set_union(const Other& other) {
    thes::set_union(data_, other, compare_, [](const auto& p) -> const auto& { return p.first; });
  }

  void clear() {
    data_.clear();
  }

private:
  Container data_{};
  [[no_unique_address]] KCmp compare_{};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_FLAT_MAP_HPP
