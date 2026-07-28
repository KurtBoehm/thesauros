// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_RANGES_INDICES_HPP
#define INCLUDE_THESAUROS_RANGES_INDICES_HPP

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "thesauros/iterator/facade.hpp"
#include "thesauros/iterator/state-facade.hpp"
#include "thesauros/types/value-tag.hpp"

namespace thes::ranges {
namespace detail::iota {
template<typename T>
struct ConstIterator : public StateIteratorFacade<iter::ValueTypes<T, std::ptrdiff_t>> {
  friend StateIteratorFacade<iter::ValueTypes<T, std::ptrdiff_t>>;

  constexpr ConstIterator() = default;
  explicit constexpr ConstIterator(T&& v) : value_(std::forward<T>(v)) {}
  explicit constexpr ConstIterator(const T& v) : value_(v) {}

private:
  constexpr T value(this const auto& self) {
    return self.value_;
  }
  constexpr auto& state(this auto& self) {
    return self.value_;
  }

  T value_{};
};

template<typename T>
struct ConstReverseIterator : public std::reverse_iterator<ConstIterator<T>> {
  using ForwardIter = ConstIterator<T>;
  using Base = std::reverse_iterator<ConstIterator<T>>;

  constexpr ConstReverseIterator() = default;
  explicit constexpr ConstReverseIterator(T&& v) : Base{ForwardIter{std::forward<T>(v)}} {}
  explicit constexpr ConstReverseIterator(const T& v) : Base{ForwardIter{v}} {}
};
} // namespace detail::iota

template<typename T>
struct IotaRange {
  using Value = T;
  using value_type = Value;
  using const_iterator = detail::iota::ConstIterator<T>;
  using const_reverse_iterator = detail::iota::ConstReverseIterator<T>;

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

  struct ConstIterator {
    Value value;
    const Value step;

    bool operator==(const Sentinel& other) const {
      return value >= other.end;
    }

    ConstIterator& operator++() {
      value += step;
      return *this;
    }
    Value operator*() const {
      return value;
    }
  };

  using const_iterator = ConstIterator;

  ConstIterator begin() const {
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

template<typename T, T Size>
struct StaticSizeIotaRange {
  using Value = T;
  using value_type = Value;
  using const_iterator = detail::iota::ConstIterator<T>;
  using const_reverse_iterator = detail::iota::ConstReverseIterator<T>;

  explicit constexpr StaticSizeIotaRange(T begin) : begin_{begin} {}
  explicit constexpr StaticSizeIotaRange(T begin, ValueTag<T, Size> /*tag*/) : begin_{begin} {}

  [[nodiscard]] constexpr const_iterator begin() const {
    return const_iterator{begin_};
  }
  [[nodiscard]] constexpr const_iterator end() const {
    return const_iterator{begin_ + Size};
  }

  [[nodiscard]] constexpr const_reverse_iterator rbegin() const {
    return const_reverse_iterator{begin_ + Size};
  }
  [[nodiscard]] constexpr const_reverse_iterator rend() const {
    return const_reverse_iterator{begin_};
  }

  [[nodiscard]] constexpr Value begin_value() const {
    return begin_;
  }
  [[nodiscard]] constexpr Value end_value() const {
    return begin_ + Size;
  }
  [[nodiscard]] constexpr auto size() const {
    return Size;
  }

private:
  T begin_;
};
template<typename T, T Size>
StaticSizeIotaRange(T, ValueTag<T, Size>) -> StaticSizeIotaRange<T, Size>;

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

template<typename Range>
constexpr IotaInfo<typename IotaTrait<Range>::Value> iota_info(const Range& range) {
  using Trait = IotaTrait<Range>;
  using Value = Trait::Value;
  return IotaInfo<Value>{Trait::front(range), Trait::bound(range)};
}
} // namespace thes::ranges

namespace thes::views {
/** The half-open range of indices `[begin, end)`, advancing by `step`. */
template<typename T>
constexpr ranges::ExtendedIotaRange<T> indices(T begin, T end, T step) {
  return {begin, end, step};
}
/** The half-open range of indices `[begin, end)`, empty if `end` precedes `begin`. */
template<typename T>
constexpr ranges::IotaRange<T> indices(T begin, T end) {
  return {begin, std::max(begin, end)};
}
/** The half-open range of indices `[T{}, end)`. */
template<typename T>
constexpr ranges::IotaRange<T> indices(T end) {
  return indices(T(), end);
}

/** The range of `size` consecutive indices starting at `begin`. */
template<std::integral T>
constexpr ranges::IotaRange<T> indices_n(T begin, T size) {
  return {begin, begin + size};
}
/**
 * The range of `Size` consecutive indices starting at `begin`, with `Size` a compile-time value.
 */
template<std::integral T, T Size>
constexpr ranges::StaticSizeIotaRange<T, Size> indices_n(T begin, ValueTag<T, Size> /*tag*/) {
  return ranges::StaticSizeIotaRange<T, Size>{begin};
}
} // namespace thes::views

#endif // INCLUDE_THESAUROS_RANGES_INDICES_HPP
