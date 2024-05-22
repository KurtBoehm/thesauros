#ifndef INCLUDE_THESAUROS_CONTAINERS_FLAT_SET_HPP
#define INCLUDE_THESAUROS_CONTAINERS_FLAT_SET_HPP

#include <algorithm>
#include <cstddef>
#include <functional>

#include "thesauros/containers/array/dynamic.hpp"
#include "thesauros/containers/set-algorithms.hpp"

namespace thes {
template<typename TValue, typename TCompare = std::less<TValue>,
         typename TEqual = std::equal_to<TValue>, typename TContainer = DynamicArray<TValue>>
struct FlatSet {
  using Value = TValue;
  using Container = TContainer;

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

  const TValue& front() const {
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

  void insert(const TValue& value) {
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
  [[no_unique_address]] TCompare compare_{};
  [[no_unique_address]] TEqual equal_{};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_FLAT_SET_HPP
