// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_CONCEPTS_TYPE_TRAITS_HPP
#define INCLUDE_THESAUROS_CONCEPTS_TYPE_TRAITS_HPP

#include <concepts>
#include <cstddef>
#include <type_traits>

namespace thes {
template<typename T>
concept CharacterType =
  std::same_as<T, signed char> || std::same_as<T, unsigned char> || std::same_as<T, char> ||
  std::same_as<T, wchar_t> || std::same_as<T, char8_t> || std::same_as<T, char16_t> ||
  std::same_as<T, char32_t>;

template<typename T>
concept ConstAccess = std::is_const_v<std::remove_reference_t<T>>;

template<typename T, std::size_t tSize = 1>
struct IsCompleteTrait : public std::false_type {};

template<typename T>
struct IsCompleteTrait<T, sizeof(T) / sizeof(T)> : public std::true_type {};

template<typename T>
concept CompleteType = IsCompleteTrait<T>::value;
} // namespace thes

#endif // INCLUDE_THESAUROS_CONCEPTS_TYPE_TRAITS_HPP
