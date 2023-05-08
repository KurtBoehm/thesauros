#ifndef INCLUDE_THESAUROS_UTILITY_COMBINE_TYPES_HPP
#define INCLUDE_THESAUROS_UTILITY_COMBINE_TYPES_HPP

#include <concepts>

namespace thes {
template<typename T1, typename T2>
struct UnionTrait;

template<typename T>
struct UnionTrait<T, T> {
  using Type = T;
};

template<typename T1, typename T2>
requires(!std::same_as<T1, T2> && std::unsigned_integral<T1> && std::unsigned_integral<T2>)
struct UnionTrait<T1, T2> {
  using Type = std::conditional_t<(sizeof(T1) > sizeof(T2)), T1, T2>;
};

template<typename T1, typename T2>
requires(!std::same_as<T1, T2> && std::signed_integral<T1> && std::signed_integral<T2>)
struct UnionTrait<T1, T2> {
  using Type = std::conditional_t<(sizeof(T1) > sizeof(T2)), T1, T2>;
};

template<typename T1, typename T2>
using Union = typename UnionTrait<T1, T2>::Type;

template<typename T1, typename T2>
struct IntersectionTrait;

template<typename T>
struct IntersectionTrait<T, T> {
  using Type = T;
};

template<typename T1, typename T2>
requires(!std::same_as<T1, T2> && std::unsigned_integral<T1> && std::unsigned_integral<T2>)
struct IntersectionTrait<T1, T2> {
  using Type = std::conditional_t<(sizeof(T1) < sizeof(T2)), T1, T2>;
};

template<typename T1, typename T2>
requires(!std::same_as<T1, T2> && std::signed_integral<T1> && std::signed_integral<T2>)
struct IntersectionTrait<T1, T2> {
  using Type = std::conditional_t<(sizeof(T1) < sizeof(T2)), T1, T2>;
};

template<typename T1, typename T2>
using Intersection = typename IntersectionTrait<T1, T2>::Type;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_COMBINE_TYPES_HPP
