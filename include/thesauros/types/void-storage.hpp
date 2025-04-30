// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_TYPES_VOID_STORAGE_HPP
#define INCLUDE_THESAUROS_TYPES_VOID_STORAGE_HPP

#include <concepts>
#include <type_traits>

#include "thesauros/utility/empty.hpp"

namespace thes {
template<typename T, template<typename> typename... TTrans>
struct ApplyTypeTransformationsTrait;
template<typename T>
struct ApplyTypeTransformationsTrait<T> {
  using Type = T;
};
template<typename T, template<typename> typename TTrans, template<typename> typename... TOtherTrans>
struct ApplyTypeTransformationsTrait<T, TTrans, TOtherTrans...> {
  using Type = ApplyTypeTransformationsTrait<typename TTrans<T>::type, TOtherTrans...>::Type;
};

template<typename T, template<typename> typename... TTrans>
struct VoidTypeTrait {
  using Type = ApplyTypeTransformationsTrait<T, TTrans...>::Type;
  using Storage = Type;
};
template<template<typename> typename... TTrans>
struct VoidTypeTrait<void, TTrans...> {
  using Type = void;
  using Storage = Empty;
};

template<typename T>
using VoidStorage = VoidTypeTrait<T>::Storage;

template<typename T>
using VoidRvalRef = VoidTypeTrait<T, std::add_rvalue_reference>::Type;
template<typename T>
using VoidStorageRvalRef = VoidTypeTrait<T, std::add_rvalue_reference>::Storage;

template<typename T>
using VoidLvalRef = VoidTypeTrait<T, std::add_lvalue_reference>::Type;
template<typename T>
using VoidStorageLvalRef = VoidTypeTrait<T, std::add_lvalue_reference>::Storage;

template<typename T>
using VoidConstLvalRef = VoidTypeTrait<T, std::add_const, std::add_lvalue_reference>::Type;
template<typename T>
using VoidStorageConstLvalRef =
  VoidTypeTrait<T, std::add_const, std::add_lvalue_reference>::Storage;

template<typename T>
using VoidConstPtr = VoidTypeTrait<T, std::add_const, std::add_pointer>::Type;
template<typename T>
using VoidStorageConstPtr = VoidTypeTrait<T, std::add_const, std::add_pointer>::Storage;

template<typename T>
using UnVoidStorage = std::conditional_t<std::same_as<std::decay_t<T>, Empty>, void, T>;

template<typename T>
inline constexpr VoidStorageConstLvalRef<UnVoidStorage<T>> void_storage_cref(const T& value) {
  return value;
}
template<typename T>
inline constexpr VoidStorageConstPtr<UnVoidStorage<T>> void_storage_cptr(const T& value) {
  if constexpr (std::is_void_v<UnVoidStorage<T>>) {
    return value;
  } else {
    return &value;
  }
}
} // namespace thes

#endif // INCLUDE_THESAUROS_TYPES_VOID_STORAGE_HPP
