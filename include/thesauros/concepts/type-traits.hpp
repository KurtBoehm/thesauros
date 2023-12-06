#ifndef INCLUDE_THESAUROS_CONCEPTS_TYPE_TRAITS_HPP
#define INCLUDE_THESAUROS_CONCEPTS_TYPE_TRAITS_HPP

#include <cstddef>
#include <type_traits>

namespace thes {
template<typename T, std::size_t tSize = 1>
struct IsCompleteTrait : public std::false_type {};

template<typename T>
struct IsCompleteTrait<T, sizeof(T) / sizeof(T)> : public std::true_type {};

template<typename T>
concept CompleteType = IsCompleteTrait<T>::value;
} // namespace thes

#endif // INCLUDE_THESAUROS_CONCEPTS_TYPE_TRAITS_HPP
