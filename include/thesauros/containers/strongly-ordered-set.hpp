#ifndef INCLUDE_THESAUROS_CONTAINERS_STRONGLY_ORDERED_SET_HPP
#define INCLUDE_THESAUROS_CONTAINERS_STRONGLY_ORDERED_SET_HPP

#include <algorithm>
#include <functional>

#include "thesauros/containers/array/dynamic.hpp"
#include "thesauros/containers/set-algorithms.hpp"

namespace thes {
template<typename TValue, typename TKeyCompare = std::less<TValue>,
         typename TKeyEqual = std::equal_to<TValue>, typename TAllocator = std::allocator<TValue>>
struct StronglyOrderedSet {
  using Value = TValue;
  using Data = DynamicArrayDefault<TValue, TAllocator>;

  using value_type = Value;
  using iterator = typename Data::const_iterator;
  using const_iterator = typename Data::const_iterator;

  StronglyOrderedSet() = default;

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

  const TValue& front() const {
    return data_.front();
  }
  void pop_front() {
    data_.erase(data_.begin());
  }

  const_iterator lower_bound(const auto& value) const {
    return std::lower_bound(data_.begin(), data_.end(), value, TKeyCompare{});
  }

  bool contains(const auto& value) const {
    const auto it{lower_bound(value)};
    return it != end() && TKeyEqual{}(*it, value);
  }

  const_iterator find(const auto& value) const {
    const auto it{lower_bound(value)};
    if (it != end() && TKeyEqual{}(*it, value)) {
      return it;
    }
    return end();
  }

  void insert(const TValue& value) {
    const auto it{lower_bound(value)};
    if (it != end() && TKeyEqual{}(*it, value)) {
      return;
    }
    data_.insert(it, value);
  }

  bool erase(const auto& value) {
    const auto it{lower_bound(value)};
    if (it != end() && TKeyEqual{}(*it, value)) {
      data_.erase(it);
      return true;
    }
    return false;
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
    thes::set_union(data_, other, TKeyCompare{}, TKeyEqual{});
  }

  void clear() {
    data_.clear();
  }

private:
  Data data_{};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_STRONGLY_ORDERED_SET_HPP
