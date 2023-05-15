#ifndef INCLUDE_THESAUROS_UTILITY_TYPE_TRANSFORMATIONS_HPP
#define INCLUDE_THESAUROS_UTILITY_TYPE_TRANSFORMATIONS_HPP

#include <concepts>

namespace thes {
template<bool tConst, typename T>
using ConditionalConst = std::conditional_t<tConst, const T, T>;

template<typename TFrom, typename TTo>
using TransferConst = std::conditional_t<std::is_const_v<TFrom>, const TTo, TTo>;

template<typename... Ts>
struct UnionTrait;

template<typename T>
struct UnionTrait<T> {
  using Type = T;
};
template<typename T>
struct UnionTrait<T, T> {
  using Type = T;
};
template<typename T1, typename T2, typename... Ts>
requires(sizeof...(Ts) > 0)
struct UnionTrait<T1, T2, Ts...> {
  using Type = typename UnionTrait<typename UnionTrait<T1, T2>::Type, Ts...>::Type;
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

template<typename... Ts>
using Union = typename UnionTrait<Ts...>::Type;

template<typename... Ts>
struct IntersectionTrait;

template<typename T>
struct IntersectionTrait<T> {
  using Type = T;
};
template<typename T>
struct IntersectionTrait<T, T> {
  using Type = T;
};
template<typename T1, typename T2, typename... Ts>
requires(sizeof...(Ts) > 0)
struct IntersectionTrait<T1, T2, Ts...> {
  using Type = typename IntersectionTrait<typename IntersectionTrait<T1, T2>::Type, Ts...>::Type;
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

template<typename... Ts>
using Intersection = typename IntersectionTrait<Ts...>::Type;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_TYPE_TRANSFORMATIONS_HPP
