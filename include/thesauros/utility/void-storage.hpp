#ifndef INCLUDE_THESAUROS_UTILITY_VOID_STORAGE_HPP
#define INCLUDE_THESAUROS_UTILITY_VOID_STORAGE_HPP

#include <concepts>
#include <type_traits>

#include "thesauros/utility/empty.hpp"

namespace thes {
template<typename T>
struct VoidTypeTrait {
  using Type = T;
  using Cref = const T&;
  using StorageCref = const T&;
  using Cptr = const std::decay_t<T>*;
  using StorageCptr = const std::decay_t<T>*;
};
template<>
struct VoidTypeTrait<void> {
  using Type = thes::Empty;
  using Cref = void;
  using StorageCref = thes::Empty;
  using Cptr = void;
  using StorageCptr = thes::Empty;
};

template<typename T>
using VoidStorage = VoidTypeTrait<T>::Type;
template<typename T>
using VoidCref = VoidTypeTrait<T>::Cref;
template<typename T>
using VoidStorageCref = VoidTypeTrait<T>::StorageCref;
template<typename T>
using VoidCptr = VoidTypeTrait<T>::Cptr;
template<typename T>
using VoidStorageCptr = VoidTypeTrait<T>::StorageCptr;

template<typename T>
using UnVoidStorage = std::conditional_t<std::same_as<T, thes::Empty>, void, T>;

template<typename T>
inline constexpr VoidStorageCref<UnVoidStorage<T>> void_storage_cref(const T& value) {
  return value;
}
template<typename T>
inline constexpr VoidStorageCptr<UnVoidStorage<T>> void_storage_cptr(const T& value) {
  if constexpr (std::is_void_v<UnVoidStorage<T>>) {
    return value;
  } else {
    return &value;
  }
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_VOID_STORAGE_HPP
