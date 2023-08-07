#ifndef INCLUDE_THESAUROS_UTILITY_VOID_STORAGE_HPP
#define INCLUDE_THESAUROS_UTILITY_VOID_STORAGE_HPP

#include "thesauros/utility/empty.hpp"
#include <concepts>
#include <type_traits>

namespace thes {
template<typename T>
struct VoidStorageTrait {
  using Type = T;
  using Cref = const T&;
  using Rvalue = const T&;
};
template<>
struct VoidStorageTrait<void> {
  using Type = thes::Empty;
  using Cref = void;
  using Rvalue = thes::Empty;
};

template<typename T>
using VoidStorage = VoidStorageTrait<T>::Type;
template<typename T>
using VoidStorageCref = VoidStorageTrait<T>::Cref;
template<typename T>
inline constexpr VoidStorageTrait<T>::Rvalue
void_storage_rvalue(const typename VoidStorageTrait<T>::Type& value) {
  return value;
}

template<typename T>
using UnVoidStorage = std::conditional_t<std::same_as<T, thes::Empty>, void, T>;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_VOID_STORAGE_HPP
