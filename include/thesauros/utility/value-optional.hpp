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
#include <type_traits>
#include <utility>

namespace thes {
/**
 * Optional-like wrapper using a dedicated sentinel value to encode the empty state.
 *
 * The type `T` must allow storing a distinguished value `EmptyValue` that is never used for
 * non-empty values. `ValueOptional` uses `EmptyValue` internally to represent the empty state.
 */
template<typename T, T EmptyValue>
struct ValueOptional {
  using Value = T;

  /** Sentinel representing the empty state. */
  static constexpr Value empty_value = EmptyValue;

  /** Construct an empty optional (`empty_value`). */
  constexpr ValueOptional() = default;

  /** Construct an optional containing `value`. */
  constexpr ValueOptional(const Value& value) : value_{value} {}

  /** Construct an optional containing `value`. */
  constexpr ValueOptional(Value&& value) : value_{std::forward<Value>(value)} {}

  /** Set the optional to the empty state. */
  void clear() {
    value_ = empty_value;
  }

  /** Store a new value, which must not be equal to `empty_value`. */
  void set(const Value& value) {
    assert(value != empty_value);
    value_ = value;
  }

  /** Access the stored value; asserts that a value is present. */
  [[nodiscard]] const Value& value() const {
    assert(value_ != empty_value);
    return value_;
  }

  /** Dereference to the stored value; does not check for emptiness. */
  [[nodiscard]] const Value& operator*() const {
    return value_;
  }

  /** Return whether a value is present (`value_ != empty_value`). */
  [[nodiscard]] bool has_value() const {
    return value_ != empty_value;
  }

  /** Return whether the optional is empty (`value_ == empty_value`). */
  [[nodiscard]] bool is_empty() const {
    return value_ == empty_value;
  }

  /** Invoke `f(*this)` if a value is present. */
  template<typename TF>
  constexpr void value_run(TF&& f) {
    if (has_value()) {
      std::invoke(std::forward<TF>(f), **this);
    }
  }

  /**
   * Monadic bind: `has_value() ? f(value()) : empty`.
   *
   * The callable `f` is invoked only when a value is present and must return an
   * optional-like, default-initializable result.
   */
  template<typename TF>
  [[nodiscard]] constexpr auto and_then(TF&& f) const
  requires(std::invocable<TF, const Value&> &&
           std::default_initializable<std::remove_cvref_t<std::invoke_result_t<TF, const Value&>>>)
  {
    using Result = std::remove_cvref_t<std::invoke_result_t<TF, const Value&>>;

    if (has_value()) {
      return std::invoke(std::forward<TF>(f), **this);
    }

    return Result{};
  }

  /**
   * Transform the contained value using `f`, keeping the same `Value` type.
   *
   * If a value is present, returns a `ValueOptional` containing `f(value())`.
   * Otherwise returns an empty `ValueOptional`.
   *
   * Unlike `std::optional::transform`, the element type does not change, because the empty
   * state is encoded via the sentinel `empty_value`.
   */
  template<typename TF>
  [[nodiscard]] constexpr ValueOptional transform(TF&& f) const
  requires(std::invocable<TF, const Value&> &&
           std::convertible_to<std::invoke_result_t<TF, const Value&>, Value>)
  {
    if (has_value()) {
      return ValueOptional{std::invoke(std::forward<TF>(f), **this)};
    }

    return ValueOptional{};
  }

  /**
   * Fallback: `has_value() ? *this : f()`.
   *
   * The callable `f` is invoked only when the optional is empty and must produce a
   * `ValueOptional` or something convertible to it.
   */
  template<typename TF>
  [[nodiscard]] constexpr ValueOptional or_else(TF&& f) const
  requires(std::invocable<TF> && std::convertible_to<std::invoke_result_t<TF>, ValueOptional>)
  {
    if (has_value()) {
      return *this;
    }

    return std::invoke(std::forward<TF>(f));
  }

private:
  Value value_{empty_value};
};

/**
 * `ValueOptional` using the maximum representable value of an integral type as sentinel.
 *
 * This is convenient for value ranges where the maximum value is unused for non-empty data.
 * It is similar to using `std::npos` as a no-value in various container operations.
 */
template<std::integral T>
using MaxOptional = ValueOptional<T, std::numeric_limits<T>::max()>;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_VALUE_OPTIONAL_HPP
