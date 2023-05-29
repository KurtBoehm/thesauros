#ifndef INCLUDE_THESAUROS_CONTAINERS_STRONGLY_ORDERED_MAP_HPP
#define INCLUDE_THESAUROS_CONTAINERS_STRONGLY_ORDERED_MAP_HPP

#include <cstddef>
#include <functional>
#include <memory>
#include <ostream>
#include <utility>

#include "thesauros/containers/array/dynamic.hpp"
#include "thesauros/containers/set-algorithms.hpp"

namespace thes {
namespace strong_order {
template<typename TKey, typename TValue>
struct Pair {
  constexpr Pair(TKey key, TValue value) : key_{std::move(key)}, value_{std::move(value)} {}

  constexpr const TKey& key() const {
    return key_;
  }

  constexpr TValue& value() {
    return value_;
  }
  constexpr const TValue& value() const {
    return value_;
  }

  friend std::ostream& operator<<(std::ostream& stream, const Pair& value) {
    return stream << value.key() << "→" << value.value();
  }

  constexpr bool operator==(const Pair& other) const = default;
  constexpr auto operator<=>(const Pair& other) const = default;

private:
  TKey key_;
  TValue value_;
};
} // namespace strong_order

template<typename TKey, typename TMapped, typename TKeyCompare = std::less<TKey>,
         typename TKeyEqual = std::equal_to<TKey>,
         typename TAllocator = std::allocator<strong_order::Pair<TKey, TMapped>>>
struct StronglyOrderedMap {
  using Key = TKey;
  using Mapped = TMapped;
  using Allocator = TAllocator;
  using KeyCompare = TKeyCompare;
  using KeyEqual = TKeyEqual;

  using Value = strong_order::Pair<Key, Mapped>;
  using Data = DynamicArrayDefault<Value, TAllocator>;

  using value_type = Value;
  using iterator = typename Data::iterator;
  using const_iterator = typename Data::const_iterator;

  StronglyOrderedMap() = default;

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
    return std::lower_bound(data_.begin(), data_.end(), key, PairCompare{});
  }
  const_iterator lower_bound(const auto& key) const {
    return std::lower_bound(data_.begin(), data_.end(), key, PairCompare{});
  }

  bool contains(const auto& key) const {
    const auto it{lower_bound(key)};
    return it != end() && PairEqual{}(*it, key);
  }

  iterator find(const auto& key) {
    const auto it{lower_bound(key)};
    if (it != end() && PairEqual{}(*it, key)) {
      return it;
    }
    return end();
  }
  const_iterator find(const auto& key) const {
    const auto it{lower_bound(key)};
    if (it != end() && PairEqual{}(*it, key)) {
      return it;
    }
    return end();
  }

  bool insert(const Key& key, const Mapped& value) {
    const auto it{lower_bound(key)};
    if (it != end() && PairEqual{}(*it, key)) {
      return false;
    }
    data_.insert(it, Value{key, value});
    return true;
  }

  Mapped& get_or_insert(const Key& key, Mapped&& value) {
    const auto iter = lower_bound(key);
    if (iter != end() && PairEqual{}(*iter, key)) {
      return *iter;
    }
    return *data_.insert(iter, Value{key, value});
  }
  template<typename TTrans, typename TCreate>
  void transform_or_create(const Key& key, TTrans&& transform, TCreate&& create) {
    const auto iter = lower_bound(key);
    if (iter != end() && PairEqual{}(*iter, key)) {
      transform(iter->value());
      return;
    }
    data_.insert(iter, Value{key, create()});
  }

  bool erase(const auto& key) {
    const auto it{lower_bound(key)};
    if (it != end() && PairEqual{}(*it, key)) {
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
    thes::set_union(data_, other, PairCompare{}, PairEqual{});
  }

  void clear() {
    data_.clear();
  }

private:
  template<typename TOp>
  struct PairOp {
    bool operator()(const Value& first, const Value& second) const {
      return TOp{}(first.key(), second.key());
    }
    bool operator()(const Key& first, const Value& second) const {
      return TOp{}(first, second.key());
    }
    bool operator()(const Value& first, const Key& second) const {
      return TOp{}(first.key(), second);
    }
    bool operator()(const Key& first, const Key& second) const {
      return TOp{}(first, second);
    }
  };

  using PairCompare = PairOp<KeyCompare>;
  using PairEqual = PairOp<KeyEqual>;

  Data data_{};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_STRONGLY_ORDERED_MAP_HPP
