#ifndef INCLUDE_THESAUROS_RANGES_IOTA_HPP
#define INCLUDE_THESAUROS_RANGES_IOTA_HPP

#include <cstddef>

#include "thesauros/iterator/facades.hpp"
#include "thesauros/iterator/provider-map.hpp"
#include "thesauros/iterator/provider-reverse.hpp"

namespace thes {
template<typename T>
struct IotaRange {
private:
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

public:
  using Value = T;
  using value_type = Value;

  constexpr IotaRange() : begin_{}, end_{} {}
  constexpr IotaRange(T begin, T end) : begin_{begin}, end_{end} {}

  struct const_iterator
      : public IteratorFacade<const_iterator, iter_provider::Map<IterProv, const_iterator>> {
    friend IterProv;
    explicit constexpr const_iterator(Value&& v) : value_(std::move(v)) {}
    explicit constexpr const_iterator(const Value& v) : value_(v) {}

  private:
    Value value_;
  };

  struct const_reverse_iterator
      : public IteratorFacade<
          const_reverse_iterator,
          iter_provider::Reverse<iter_provider::Map<IterProv, const_reverse_iterator>,
                                 const_reverse_iterator>> {
    friend IterProv;
    explicit constexpr const_reverse_iterator(Value&& v) : value_(std::forward<Value>(v)) {}
    explicit constexpr const_reverse_iterator(const Value& v) : value_(v) {}

  private:
    Value value_;
  };

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

  [[nodiscard]] constexpr bool contains(const T& value) const {
    return begin_ <= value && value < end_;
  }

  [[nodiscard]] constexpr bool is_empty() const {
    return begin_ == end_;
  }

  [[nodiscard]] constexpr Value size() const {
    return end_ - begin_;
  }

  friend IotaRange operator&(const IotaRange& r1, const IotaRange& r2) {
    const T new_begin = std::max(r1.begin_, r2.begin_);
    const T new_end = std::min(r1.end_, r2.end_);
    return {new_begin, std::max(new_begin, new_end)};
  }

private:
  const T begin_, end_;
};

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
inline constexpr ExtendedIotaRange<T> range(T begin, T end, T step) {
  return {begin, end, step};
}
template<typename T>
inline constexpr IotaRange<T> range(T begin, T end) {
  return {begin, std::max(begin, end)};
}
template<typename T>
inline constexpr IotaRange<T> range(T end) {
  return range(T(), end);
}

template<typename TRange>
inline constexpr auto iter_range(TRange&& container) {
  return IotaRange{container.begin(), container.end()};
}
template<typename TIter>
inline constexpr auto iter_range(TIter begin, TIter end) {
  return IotaRange{std::move(begin), std::move(end)};
}
} // namespace thes

#endif // INCLUDE_THESAUROS_RANGES_IOTA_HPP
