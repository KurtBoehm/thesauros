// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_UTILITY_OPTIONAL_HPP
#define INCLUDE_THESAUROS_UTILITY_OPTIONAL_HPP

#include <concepts>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>

#include "thesauros/macropolis/inlining.hpp"

namespace thes {
template<typename T>
struct Optional;

namespace detail {
template<typename T>
struct IsOptionalTrait : public std::false_type {};
template<typename T>
struct IsOptionalTrait<Optional<T>> : public std::true_type {};
template<typename F, typename T>
concept ReturnsOptional = IsOptionalTrait<std::remove_cvref_t<std::invoke_result_t<F, T>>>::value;
} // namespace detail

template<typename T>
struct Optional : public std::optional<T> {
  using std::optional<T>::optional;

  Optional(const std::optional<T>& opt) : std::optional<T>(opt) {}
  Optional(std::optional<T>&& opt) : std::optional<T>(std::move(opt)) {}

  template<detail::ReturnsOptional<T&> F>
  constexpr auto and_then(F&& f) & {
    return this->has_value() ? std::invoke(std::forward<F>(f), **this)
                             : std::remove_cvref_t<std::invoke_result_t<F, T&>>();
  }
  template<detail::ReturnsOptional<const T&> F>
  constexpr auto and_then(F&& f) const& {
    return this->has_value() ? std::invoke(std::forward<F>(f), **this)
                             : std::remove_cvref_t<std::invoke_result_t<F, const T&>>();
  }
  template<detail::ReturnsOptional<T> F>
  constexpr auto and_then(F&& f) && {
    return this->has_value() ? std::invoke(std::forward<F>(f), std::move(**this))
                             : std::remove_cvref_t<std::invoke_result_t<F, T>>();
  }
  template<detail::ReturnsOptional<const T> F>
  constexpr auto and_then(F&& f) const&& {
    return this->has_value() ? std::invoke(std::forward<F>(f), std::move(**this))
                             : std::remove_cvref_t<std::invoke_result_t<F, const T>>();
  }

  template<typename F>
  constexpr auto transform(F&& f) & {
    using Ret = std::remove_cv_t<std::invoke_result_t<F, T&>>;
    return this->has_value() ? std::invoke(std::forward<F>(f), **this) : Optional<Ret>{};
  }
  template<typename F>
  constexpr auto transform(F&& f) const& {
    using Ret = std::remove_cv_t<std::invoke_result_t<F, T&>>;
    return this->has_value() ? std::invoke(std::forward<F>(f), **this) : Optional<Ret>{};
  }
  template<typename F>
  constexpr auto transform(F&& f) && {
    using Ret = std::remove_cv_t<std::invoke_result_t<F, T&>>;
    return this->has_value() ? std::invoke(std::forward<F>(f), std::move(**this)) : Optional<Ret>{};
  }
  template<typename F>
  constexpr auto transform(F&& f) const&& {
    using Ret = std::remove_cv_t<std::invoke_result_t<F, T&>>;
    return this->has_value() ? std::invoke(std::forward<F>(f), std::move(**this)) : Optional<Ret>{};
  }

  template<typename F>
  requires(std::same_as<std::remove_cvref_t<std::invoke_result_t<F>>, Optional> &&
           std::copy_constructible<T>)
  constexpr Optional or_else(F&& f) const& {
    return this->has_value() ? *this : std::forward<F>(f)();
  }

  template<typename F>
  requires(std::same_as<std::remove_cvref_t<std::invoke_result_t<F>>, Optional> &&
           std::move_constructible<T>)
  constexpr Optional or_else(F&& f) && {
    return this->has_value() ? std::move(*this) : std::forward<F>(f)();
  }

  template<typename F>
  requires(std::same_as<std::remove_cvref_t<std::invoke_result_t<F>>, T> &&
           std::copy_constructible<T>)
  THES_ALWAYS_INLINE constexpr T value_or_else(F&& f) const& {
    return this->has_value() ? **this : std::forward<F>(f)();
  }

  template<typename F>
  requires(std::same_as<std::remove_cvref_t<std::invoke_result_t<F>>, T> &&
           std::move_constructible<T>)
  THES_ALWAYS_INLINE constexpr T value_or_else(F&& f) && {
    return this->has_value() ? std::move(**this) : std::forward<F>(f)();
  }
};
template<typename T>
Optional(T) -> Optional<T>;
template<typename T>
Optional(std::optional<T>) -> Optional<T>;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_OPTIONAL_HPP
