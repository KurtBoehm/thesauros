#ifndef INCLUDE_THESAUROS_UTILITY_TYPE_TRANSFORMATIONS_HPP
#define INCLUDE_THESAUROS_UTILITY_TYPE_TRANSFORMATIONS_HPP

#include <concepts>
#include <limits>
#include <type_traits>

#include "thesauros/utility/fixed-size-integer.hpp"

namespace thes {
template<typename T>
struct AddConstTrait {
  using Type = const T;
};
template<typename T>
struct AddConstTrait<T&> {
  using Type = const T&;
};
template<typename T>
using AddConst = AddConstTrait<T>::Type;

template<bool tConst, typename T>
using ConditionalConst = std::conditional_t<tConst, const T, T>;

template<typename TFrom, typename TTo>
using TransferConst = std::conditional_t<std::is_const_v<TFrom>, const TTo, TTo>;

template<typename T, typename TDummy>
using First = T;

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
  using Type = UnionTrait<typename UnionTrait<T1, T2>::Type, Ts...>::Type;
};

template<std::unsigned_integral T1, std::unsigned_integral T2>
requires(!std::same_as<T1, T2>)
struct UnionTrait<T1, T2> {
  using Type = std::conditional_t<(sizeof(T1) > sizeof(T2)), T1, T2>;
};
template<std::signed_integral T1, std::signed_integral T2>
requires(!std::same_as<T1, T2>)
struct UnionTrait<T1, T2> {
  using Type = std::conditional_t<(sizeof(T1) > sizeof(T2)), T1, T2>;
};
template<std::unsigned_integral T1, std::signed_integral T2>
requires(!std::same_as<T1, T2>)
struct UnionTrait<T1, T2> {
  using Type = std::conditional_t<sizeof(T1) < sizeof(T2), T2, FixedSignedInt<2 * sizeof(T1)>>;
};
template<std::signed_integral T1, std::unsigned_integral T2>
requires(!std::same_as<T1, T2>)
struct UnionTrait<T1, T2> : public UnionTrait<T2, T1> {};
template<std::floating_point T1, std::floating_point T2>
requires(!std::same_as<T1, T2> && std::numeric_limits<T1>::is_iec559 &&
         std::numeric_limits<T2>::is_iec559)
struct UnionTrait<T1, T2> {
  using Type = std::conditional_t<(sizeof(T1) > sizeof(T2)), T1, T2>;
};

template<typename... Ts>
using Union = UnionTrait<Ts...>::Type;

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
  using Type = IntersectionTrait<typename IntersectionTrait<T1, T2>::Type, Ts...>::Type;
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
requires(!std::same_as<T1, T2> && std::floating_point<T1> && std::floating_point<T2> &&
         std::numeric_limits<T1>::is_iec559 && std::numeric_limits<T2>::is_iec559)
struct IntersectionTrait<T1, T2> {
  using Type = std::conditional_t<(sizeof(T1) < sizeof(T2)), T1, T2>;
};

template<typename... Ts>
using Intersection = IntersectionTrait<Ts...>::Type;
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_TYPE_TRANSFORMATIONS_HPP
