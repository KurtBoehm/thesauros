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
template<typename K, typename V, typename KCmp = std::less<K>, typename KEq = std::equal_to<K>,
         typename C = DynamicArray<std::pair<K, V>>>
struct FlatMap {
  using Key = K;
  using Mapped = V;
  using KeyCompare = KCmp;
  using KeyEqual = KEq;

  using Value = std::pair<Key, Mapped>;
  using Container = C;

  using value_type = Value;
  using iterator = Container::iterator;
  using const_iterator = Container::const_iterator;

  FlatMap() = default;

  iterator begin() {
    return data_.begin();
  }
  const_iterator begin() const {
    return data_.begin();
  }
  iterator end() {
    return data_.end();
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

  const Value& front() const {
    return data_.front();
  }
  Value& front() {
    return data_.front();
  }
  void pop_front() {
    data_.erase(data_.begin());
  }

  iterator lower_bound(const auto& key) {
    return std::lower_bound(data_.begin(), data_.end(), key, PairCompare{compare_});
  }
  const_iterator lower_bound(const auto& key) const {
    return std::lower_bound(data_.begin(), data_.end(), key, PairCompare{compare_});
  }

  bool contains(const auto& key) const {
    const auto it{lower_bound(key)};
    return it != end() && PairEqual{equal_}(*it, key);
  }

  iterator find(const auto& key) {
    const auto it{lower_bound(key)};
    if (it != end() && PairEqual{equal_}(*it, key)) {
      return it;
    }
    return end();
  }
  const_iterator find(const auto& key) const {
    const auto it{lower_bound(key)};
    if (it != end() && PairEqual{equal_}(*it, key)) {
      return it;
    }
    return end();
  }

  bool insert(const Key& key, const Mapped& value) {
    const auto it{lower_bound(key)};
    if (it != end() && PairEqual{equal_}(*it, key)) {
      return false;
    }
    data_.insert(it, Value{key, value});
    return true;
  }

  Mapped& get_or_insert(const Key& key, Mapped&& value) {
    const auto iter = lower_bound(key);
    if (iter != end() && PairEqual{equal_}(*iter, key)) {
      return iter->second;
    }
    return data_.insert(iter, Value{key, std::forward<Mapped>(value)})->second;
  }
  template<typename Trans, typename Create>
  void transform_or_create(const Key& key, Trans&& transform, Create&& create) {
    const auto iter = lower_bound(key);
    if (iter != end() && PairEqual{equal_}(*iter, key)) {
      std::forward<Trans>(transform)(iter->second);
      return;
    }
    data_.insert(iter, Value{key, std::forward<Create>(create)()});
  }

  bool erase(const auto& key) {
    const auto it{lower_bound(key)};
    if (it != end() && PairEqual{equal_}(*it, key)) {
      data_.erase(it);
      return true;
    }
    return false;
  }

  Mapped& at(const auto& key) {
    return find(key)->second;
  }
  const Mapped& at(const auto& key) const {
    return find(key)->second;
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
    thes::set_union(data_, other, PairCompare{compare_}, PairEqual{equal_});
  }

  void clear() {
    data_.clear();
  }

private:
  template<typename Op>
  struct PairOp {
    [[no_unique_address]] Op op_{};

    bool operator()(const Value& first, const Value& second) const {
      return op_(first.first, second.first);
    }
    bool operator()(const Key& first, const Value& second) const {
      return op_(first, second.first);
    }
    bool operator()(const Value& first, const Key& second) const {
      return op_(first.first, second);
    }
    bool operator()(const Key& first, const Key& second) const {
      return op_(first, second);
    }
  };

  using PairCompare = PairOp<KeyCompare>;
  using PairEqual = PairOp<KeyEqual>;

  Container data_{};
  [[no_unique_address]] KCmp compare_{};
  [[no_unique_address]] KEq equal_{};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_FLAT_MAP_HPP
