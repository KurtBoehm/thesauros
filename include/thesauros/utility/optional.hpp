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
template<typename TF, typename T>
concept ReturnsOptional = IsOptionalTrait<std::remove_cvref_t<std::invoke_result_t<TF, T>>>::value;
} // namespace detail

template<typename T>
struct Optional : public std::optional<T> {
  using std::optional<T>::optional;

  Optional(const std::optional<T>& opt) : std::optional<T>(opt) {}
  Optional(std::optional<T>&& opt) : std::optional<T>(std::move(opt)) {}

  template<detail::ReturnsOptional<T&> TF>
  constexpr auto and_then(TF&& f) & {
    return this->has_value() ? std::invoke(std::forward<TF>(f), **this)
                             : std::remove_cvref_t<std::invoke_result_t<TF, T&>>();
  }
  template<detail::ReturnsOptional<const T&> TF>
  constexpr auto and_then(TF&& f) const& {
    return this->has_value() ? std::invoke(std::forward<TF>(f), **this)
                             : std::remove_cvref_t<std::invoke_result_t<TF, const T&>>();
  }
  template<detail::ReturnsOptional<T> TF>
  constexpr auto and_then(TF&& f) && {
    return this->has_value() ? std::invoke(std::forward<TF>(f), std::move(**this))
                             : std::remove_cvref_t<std::invoke_result_t<TF, T>>();
  }
  template<detail::ReturnsOptional<const T> TF>
  constexpr auto and_then(TF&& f) const&& {
    return this->has_value() ? std::invoke(std::forward<TF>(f), std::move(**this))
                             : std::remove_cvref_t<std::invoke_result_t<TF, const T>>();
  }

  template<typename TF>
  constexpr auto transform(TF&& f) & {
    using Ret = std::remove_cv_t<std::invoke_result_t<TF, T&>>;
    return this->has_value() ? std::invoke(std::forward<TF>(f), **this) : Optional<Ret>{};
  }
  template<typename TF>
  constexpr auto transform(TF&& f) const& {
    using Ret = std::remove_cv_t<std::invoke_result_t<TF, T&>>;
    return this->has_value() ? std::invoke(std::forward<TF>(f), **this) : Optional<Ret>{};
  }
  template<typename TF>
  constexpr auto transform(TF&& f) && {
    using Ret = std::remove_cv_t<std::invoke_result_t<TF, T&>>;
    return this->has_value() ? std::invoke(std::forward<TF>(f), std::move(**this))
                             : Optional<Ret>{};
  }
  template<typename TF>
  constexpr auto transform(TF&& f) const&& {
    using Ret = std::remove_cv_t<std::invoke_result_t<TF, T&>>;
    return this->has_value() ? std::invoke(std::forward<TF>(f), std::move(**this))
                             : Optional<Ret>{};
  }

  template<typename TF>
  requires(std::same_as<std::remove_cvref_t<std::invoke_result_t<TF>>, Optional> &&
           std::copy_constructible<T>)
  constexpr Optional or_else(TF&& f) const& {
    return this->has_value() ? *this : std::forward<TF>(f)();
  }

  template<typename TF>
  requires(std::same_as<std::remove_cvref_t<std::invoke_result_t<TF>>, Optional> &&
           std::move_constructible<T>)
  constexpr Optional or_else(TF&& f) && {
    return this->has_value() ? std::move(*this) : std::forward<TF>(f)();
  }

  template<typename TF>
  requires(std::same_as<std::remove_cvref_t<std::invoke_result_t<TF>>, T> &&
           std::copy_constructible<T>)
  THES_ALWAYS_INLINE constexpr T value_or_else(TF&& f) const& {
    return this->has_value() ? **this : std::forward<TF>(f)();
  }

  template<typename TF>
  requires(std::same_as<std::remove_cvref_t<std::invoke_result_t<TF>>, T> &&
           std::move_constructible<T>)
  THES_ALWAYS_INLINE constexpr T value_or_else(TF&& f) && {
    return this->has_value() ? std::move(**this) : std::forward<TF>(f)();
  }
};
template<typename T>
Optional(T) -> Optional<T>;
template<typename T>
Optional(std::optional<T>) -> Optional<T>;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_OPTIONAL_HPP
