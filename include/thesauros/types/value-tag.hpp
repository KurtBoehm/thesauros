// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_TYPES_VALUE_TAG_HPP
#define INCLUDE_THESAUROS_TYPES_VALUE_TAG_HPP

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
using AutoTag = ValueTag<std::decay_t<decltype(tValue)>, tValue>;
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

using TrueTag = BoolTag<true>;
inline constexpr TrueTag true_tag{};
using FalseTag = BoolTag<false>;
inline constexpr FalseTag false_tag{};

template<typename TValueTag>
struct AnyValueTagTrait : public std::false_type {};
template<typename T, T tValue>
struct AnyValueTagTrait<ValueTag<T, tValue>> : public std::true_type {};
template<typename TValueTag>
concept AnyValueTag = AnyValueTagTrait<TValueTag>::value;
template<typename TValueTag>
concept DerivedValueTag = !AnyValueTag<TValueTag> && requires {
  typename TValueTag::Value;
  requires std::same_as<std::decay_t<decltype(TValueTag::value)>, typename TValueTag::Value>;
} && std::derived_from<TValueTag, ValueTag<typename TValueTag::Value, TValueTag::value>>;
template<typename TValueTag, typename T>
concept TypedValueTag = AnyValueTag<TValueTag> && std::same_as<typename TValueTag::Value, T>;
template<typename TValueTag>
concept AnyIndexTag = TypedValueTag<TValueTag, std::size_t>;
template<typename TValueTag>
concept AnyBoolTag = TypedValueTag<TValueTag, bool>;

template<AnyValueTag TTag1, AnyValueTag TTag2>
requires std::same_as<typename TTag1::Value, typename TTag2::Value>
constexpr bool operator==(TTag1 tag1, TTag2 tag2) {
  return tag1.value == tag2.value;
}
template<DerivedValueTag TTag1, DerivedValueTag TTag2>
requires std::same_as<typename TTag1::Value, typename TTag2::Value>
constexpr bool operator==(TTag1 tag1, TTag2 tag2) = delete;
} // namespace thes

#endif // INCLUDE_THESAUROS_TYPES_VALUE_TAG_HPP
