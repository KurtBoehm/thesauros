#ifndef INCLUDE_THESAUROS_UTILITY_VALUE_TAG_HPP
#define INCLUDE_THESAUROS_UTILITY_VALUE_TAG_HPP

#include <concepts>
#include <cstddef>
#include <type_traits>

namespace thes {
template<typename T, T tValue>
struct ValueTag {
  using Value = T;
  using Self = ValueTag;
  static constexpr Value value = tValue;

  constexpr operator Value() const noexcept {
    return value;
  }
  constexpr Value operator()() const noexcept {
    return value;
  }
};
template<auto tValue>
using AutoTag = ValueTag<decltype(tValue), tValue>;
template<std::size_t tValue>
using IndexTag = AutoTag<tValue>;
template<bool tValue>
using BoolTag = AutoTag<tValue>;

template<typename T, T tValue>
inline constexpr ValueTag<T, tValue> value_tag{};
template<auto tValue>
inline constexpr AutoTag<tValue> auto_tag{};
template<std::size_t tValue>
inline constexpr IndexTag<tValue> index_tag{};
template<bool tValue>
inline constexpr BoolTag<tValue> bool_tag{};

template<typename TValueTag>
struct AnyValueTagTrait : public std::false_type {};
template<typename T, T tValue>
struct AnyValueTagTrait<ValueTag<T, tValue>> : public std::true_type {};
template<typename TValueTag>
concept AnyValueTag = AnyValueTagTrait<TValueTag>::value;
template<typename TValueTag, typename T>
concept TypedValueTag = AnyValueTag<TValueTag> && std::same_as<typename TValueTag::Value, T>;
template<typename TValueTag>
concept AnyIndexTag = TypedValueTag<TValueTag, std::size_t>;
template<typename TValueTag>
concept AnyBoolTag = TypedValueTag<TValueTag, bool>;

template<typename T, T tVal1, T tVal2>
inline constexpr bool operator==(ValueTag<T, tVal1> /*tag1*/, ValueTag<T, tVal2> /*tag2*/) {
  return tVal1 == tVal2;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_VALUE_TAG_HPP
