// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_VALUE_OPTIONAL_HPP
#define INCLUDE_THESAUROS_UTILITY_VALUE_OPTIONAL_HPP

#include <cassert>
#include <concepts>
#include <functional>
#include <limits>
#include <utility>

namespace thes {
template<typename T, T tEmpty>
struct ValueOptional {
  using Value = T;
  static constexpr Value empty_value = tEmpty;

  constexpr ValueOptional() = default;
  constexpr ValueOptional(const Value& value) : value_{value} {}
  constexpr ValueOptional(Value&& value) : value_{std::forward<Value>(value)} {}

  void clear() {
    value_ = empty_value;
  }
  void set(const Value& value) {
    assert(value != empty_value);
    value_ = value;
  }
  const Value& value() const {
    assert(value_ != empty_value);
    return value_;
  }
  const Value& operator*() const {
    return value_;
  }

  [[nodiscard]] bool has_value() const {
    return value_ != empty_value;
  }
  [[nodiscard]] bool is_empty() const {
    return value_ == empty_value;
  }

  template<typename TF>
  constexpr void value_run(TF&& f) {
    if (this->has_value()) {
      std::invoke(std::forward<TF>(f), **this);
    }
  }
  // has_value() → f(value()), otherwise → alt()
  template<typename TF, typename TAlt>
  constexpr auto value_run(TF&& f, TAlt&& alt) {
    if (this->has_value()) [[likely]] {
      return std::invoke(std::forward<TF>(f), **this);
    }
    return std::invoke(std::forward<TAlt>(alt));
  }

private:
  Value value_{empty_value};
};

template<std::integral T>
using MaxOptional = ValueOptional<T, std::numeric_limits<T>::max()>;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_VALUE_OPTIONAL_HPP
