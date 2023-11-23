#ifndef INCLUDE_THESAUROS_UTILITY_TYPE_TAG_HPP
#define INCLUDE_THESAUROS_UTILITY_TYPE_TAG_HPP

#include <type_traits>
#include <utility>

namespace thes {
template<typename T>
struct TypeTag {
  using Type = T;

  template<typename... TArgs>
  requires(std::is_constructible_v<T, TArgs...>)
  constexpr T construct(TArgs&&... args) const {
    return T(std::forward<TArgs>(args)...);
  }
};

template<typename T>
inline constexpr TypeTag<T> type_tag{};

template<typename T>
struct IsTypeTagTrait : public std::false_type {};
template<typename T>
struct IsTypeTagTrait<TypeTag<T>> : public std::true_type {};

template<typename T>
concept AnyTypeTag = IsTypeTagTrait<T>::value;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_TYPE_TAG_HPP
