#ifndef INCLUDE_THESAUROS_CONTAINERS_FLAT_MAP_HPP
#define INCLUDE_THESAUROS_CONTAINERS_FLAT_MAP_HPP

#include <cstddef>
#include <functional>
#include <utility>

#include "thesauros/containers/array/dynamic.hpp"
#include "thesauros/containers/set-algorithms.hpp"

namespace thes {
template<typename TKey, typename TMapped, typename TKeyCompare = std::less<TKey>,
         typename TKeyEqual = std::equal_to<TKey>,
         typename TContainer = DynamicArrayDefault<std::pair<TKey, TMapped>>>
struct FlatMap {
  using Key = TKey;
  using Mapped = TMapped;
  using KeyCompare = TKeyCompare;
  using KeyEqual = TKeyEqual;

  using Value = std::pair<Key, Mapped>;
  using Container = TContainer;

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
      return *iter;
    }
    return *data_.insert(iter, Value{key, value});
  }
  template<typename TTrans, typename TCreate>
  void transform_or_create(const Key& key, TTrans&& transform, TCreate&& create) {
    const auto iter = lower_bound(key);
    if (iter != end() && PairEqual{equal_}(*iter, key)) {
      std::forward<TTrans>(transform)(iter->value());
      return;
    }
    data_.insert(iter, Value{key, std::forward<TCreate>(create)()});
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
    return find(key)->value();
  }
  const Mapped& at(const auto& key) const {
    return find(key)->value();
  }

  template<typename TOther>
  void set_difference(const TOther& other) {
    thes::set_difference(data_, other);
  }

  template<typename TPred>
  void erase_if(TPred pred) {
    thes::erase_if(data_.begin(), data_.end(), pred);
  }

  template<typename TOther>
  void set_union(const TOther& other) {
    thes::set_union(data_, other, PairCompare{compare_}, PairEqual{equal_});
  }

  void clear() {
    data_.clear();
  }

private:
  template<typename TOp>
  struct PairOp {
    [[no_unique_address]] TOp op_{};

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
  [[no_unique_address]] TKeyCompare compare_{};
  [[no_unique_address]] TKeyEqual equal_{};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_FLAT_MAP_HPP
