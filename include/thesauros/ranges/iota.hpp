// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_RANGES_IOTA_HPP
#define INCLUDE_THESAUROS_RANGES_IOTA_HPP

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/iterator/facades.hpp"
#include "thesauros/iterator/provider-map.hpp"
#include "thesauros/iterator/provider-reverse.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes {
namespace iota_impl {
template<typename T>
struct IterProv {
  using Value = T;
  using State = T;
  struct IterTypes : public iter_provider::ValueTypes<Value, std::ptrdiff_t> {
    using IterState = State;
  };

  static constexpr Value deref(const auto& self) {
    return self.value_;
  }

  static constexpr State& state(auto& self) {
    return self.value_;
  }
  static constexpr const State& state(const auto& self) {
    return self.value_;
  }
};

template<typename T>
struct ConstIterator : public IteratorFacade<ConstIterator<T>, iter_provider::Map<IterProv<T>>> {
  friend IterProv<T>;
  constexpr ConstIterator() = default;
  explicit constexpr ConstIterator(T&& v) : value_(std::forward<T>(v)) {}
  explicit constexpr ConstIterator(const T& v) : value_(v) {}

private:
  T value_{};
};

template<typename T>
struct ConstReverseIterator
    : public IteratorFacade<
        ConstReverseIterator<T>,
        iter_provider::Reverse<iter_provider::Map<IterProv<T>>, ConstReverseIterator<T>>> {
  friend IterProv<T>;
  constexpr ConstReverseIterator() = default;
  explicit constexpr ConstReverseIterator(T&& v) : value_(std::forward<T>(v)) {}
  explicit constexpr ConstReverseIterator(const T& v) : value_(v) {}

private:
  T value_{};
};
} // namespace iota_impl

template<typename T>
struct IotaRange {
  using Value = T;
  using value_type = Value;
  using const_iterator = iota_impl::ConstIterator<T>;
  using const_reverse_iterator = iota_impl::ConstReverseIterator<T>;

  constexpr IotaRange() : begin_{}, end_{} {}
  constexpr IotaRange(T begin, T end) : begin_{begin}, end_{end} {}

  [[nodiscard]] constexpr Value begin_value() const {
    return begin_;
  }
  [[nodiscard]] constexpr Value end_value() const {
    return end_;
  }

  [[nodiscard]] constexpr const_iterator begin() const {
    return const_iterator{begin_};
  }
  [[nodiscard]] constexpr const_iterator end() const {
    return const_iterator{end_};
  }

  [[nodiscard]] constexpr const_reverse_iterator rbegin() const {
    return const_reverse_iterator{end_};
  }
  [[nodiscard]] constexpr const_reverse_iterator rend() const {
    return const_reverse_iterator{begin_};
  }

  [[nodiscard]] constexpr Value operator[](Value i) const {
    return begin_ + i;
  }

  [[nodiscard]] constexpr bool contains(const T& value) const {
    return begin_ <= value && value < end_;
  }
  [[nodiscard]] constexpr bool valid_offset(const T& value) const {
    return begin_ <= value && value <= end_;
  }

  [[nodiscard]] constexpr bool is_empty() const {
    return begin_ == end_;
  }

  [[nodiscard]] constexpr auto size() const {
    if constexpr (std::integral<T>) {
      return static_cast<T>(end_ - begin_);
    } else {
      return end_ - begin_;
    }
  }

  constexpr bool operator==(const IotaRange& other) const = default;

  auto transform(auto op) const {
    return IotaRange{op(begin_), op(end_)};
  }

  friend IotaRange operator&(const IotaRange& r1, const IotaRange& r2) {
    const T new_begin = std::max(r1.begin_, r2.begin_);
    const T new_end = std::min(r1.end_, r2.end_);
    return {std::min(new_begin, new_end), new_end};
  }

  friend IotaRange operator|(IotaRange r1, IotaRange r2) {
    assert(r1.begin_ <= r2.end_ && r2.begin_ <= r1.end_);
    return IotaRange{std::min(r1.begin_, r2.begin_), std::max(r1.end_, r2.end_)};
  }

private:
  T begin_;
  T end_;
};
template<typename T>
IotaRange(T, T) -> IotaRange<T>;

template<typename T>
struct IsIotaRange : public std::false_type {};
template<typename T>
struct IsIotaRange<IotaRange<T>> : public std::true_type {};
template<typename T>
concept AnyIotaRange = IsIotaRange<T>::value;

template<typename T>
struct ExtendedIotaRange {
  using Value = T;
  using value_type = Value;

  ExtendedIotaRange(Value begin, Value end, Value step) : begin_{begin}, end_{end}, step_{step} {}

  struct Sentinel {
    const Value end;
  };

  struct const_iterator {
    Value value;
    const Value step;

    bool operator==(const Sentinel& other) const {
      return value >= other.end;
    }

    const_iterator& operator++() {
      value += step;
      return *this;
    }
    Value operator*() const {
      return value;
    }
  };

  const_iterator begin() const {
    return {begin_, step_};
  }
  Sentinel end() const {
    return {end_};
  }

  bool contains(const Value& value) const {
    return begin_ <= value && value < end_ && ((value - begin_) % step_ == 0);
  }

private:
  const Value begin_, end_, step_;
};

template<typename T>
constexpr ExtendedIotaRange<T> range(T begin, T end, T step) {
  return {begin, end, step};
}
template<typename T>
constexpr IotaRange<T> range(T begin, T end) {
  return {begin, std::max(begin, end)};
}
template<typename T>
constexpr IotaRange<T> range(T end) {
  return range(T(), end);
}

template<typename TRange>
constexpr auto iter_range(TRange&& container) {
  return IotaRange{container.begin(), container.end()};
}
template<typename TIter>
constexpr auto iter_range(TIter begin, TIter end) {
  return IotaRange{std::move(begin), std::move(end)};
}

template<typename T, T tSize>
struct StaticSizeIotaRange {
  using Value = T;
  using value_type = Value;
  using const_iterator = iota_impl::ConstIterator<T>;
  using const_reverse_iterator = iota_impl::ConstReverseIterator<T>;

  explicit StaticSizeIotaRange(T begin) : begin_{begin} {}
  explicit StaticSizeIotaRange(T begin, ValueTag<T, tSize> /*tag*/) : begin_{begin} {}

  [[nodiscard]] constexpr const_iterator begin() const {
    return const_iterator{begin_};
  }
  [[nodiscard]] constexpr const_iterator end() const {
    return const_iterator{begin_ + tSize};
  }

  [[nodiscard]] constexpr const_reverse_iterator rbegin() const {
    return const_reverse_iterator{begin_ + tSize};
  }
  [[nodiscard]] constexpr const_reverse_iterator rend() const {
    return const_reverse_iterator{begin_};
  }

  [[nodiscard]] constexpr Value begin_value() const {
    return begin_;
  }
  [[nodiscard]] constexpr Value end_value() const {
    return begin_ + tSize;
  }
  [[nodiscard]] constexpr auto size() const {
    return tSize;
  }

private:
  T begin_;
};
template<typename T, T tSize>
StaticSizeIotaRange(T, ValueTag<T, tSize>) -> StaticSizeIotaRange<T, tSize>;

template<std::integral T>
constexpr IotaRange<T> range_size(T begin, T size) {
  return {begin, begin + size};
}
template<std::integral T, T tSize>
constexpr StaticSizeIotaRange<T, tSize> range_size(T begin, ValueTag<T, tSize> /*tag*/) {
  return StaticSizeIotaRange<T, tSize>{begin};
}

template<typename T>
struct IotaTrait;
template<typename T>
struct IotaTrait<IotaRange<T>> {
  using Value = T;

  static constexpr T front(const IotaRange<T>& r) {
    return r.begin_value();
  }
  static constexpr T bound(const IotaRange<T>& r) {
    return r.end_value();
  }
};

template<typename T>
struct IotaInfo {
  T front;
  T bound;

  [[nodiscard]] constexpr std::pair<T, T> range() const {
    return {front, bound};
  }

  [[nodiscard]] constexpr auto size() const {
    if constexpr (requires(T& mref, const T& cref) { mref -= cref; }) {
      T out = bound;
      out -= front;
      return out;
    } else {
      return bound - front;
    }
  }
};

template<typename TRange>
constexpr IotaInfo<typename IotaTrait<TRange>::Value> iota_info(const TRange& range) {
  using Trait = IotaTrait<TRange>;
  using Value = Trait::Value;
  return IotaInfo<Value>{Trait::front(range), Trait::bound(range)};
}
} // namespace thes

#endif // INCLUDE_THESAUROS_RANGES_IOTA_HPP
