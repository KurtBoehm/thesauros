#ifndef INCLUDE_THESAUROS_UTILITY_VOID_STORAGE_HPP
#define INCLUDE_THESAUROS_UTILITY_VOID_STORAGE_HPP

#include "thesauros/utility/empty.hpp"
#include <concepts>
#include <type_traits>

namespace thes {
template<typename T>
struct VoidTypeTrait {
  using Type = T;
  using Cref = const T&;
  using StorageCref = const T&;
};
template<>
struct VoidTypeTrait<void> {
  using Type = thes::Empty;
  using Cref = void;
  using StorageCref = thes::Empty;
};

template<typename T>
using VoidStorage = VoidTypeTrait<T>::Type;
template<typename T>
using VoidCref = VoidTypeTrait<T>::Cref;
template<typename T>
using VoidStorageCref = VoidTypeTrait<T>::StorageCref;

template<typename T>
inline constexpr VoidStorageCref<T> void_storage_cref(const VoidStorage<T>& value) {
  return value;
}

template<typename T>
using UnVoidStorage = std::conditional_t<std::same_as<T, thes::Empty>, void, T>;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_VOID_STORAGE_HPP
