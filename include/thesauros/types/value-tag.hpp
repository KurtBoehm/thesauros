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
template<typename T, T V>
struct ValueTag {
  using Value = T;
  using Self = ValueTag;
  static constexpr Value value = V;

  constexpr operator Value() const noexcept {
    return value;
  }
  constexpr Value operator()() const noexcept {
    return value;
  }
};
template<auto V>
using AutoTag = ValueTag<std::decay_t<decltype(V)>, V>;
template<std::size_t V>
using IndexTag = AutoTag<V>;
template<bool V>
using BoolTag = AutoTag<V>;

template<typename T, T V>
inline constexpr ValueTag<T, V> value_tag{};
template<auto V>
inline constexpr AutoTag<V> auto_tag{};
template<std::size_t V>
inline constexpr IndexTag<V> index_tag{};
template<bool V>
inline constexpr BoolTag<V> bool_tag{};

using TrueTag = BoolTag<true>;
inline constexpr TrueTag true_tag{};
using FalseTag = BoolTag<false>;
inline constexpr FalseTag false_tag{};

template<typename VTag>
struct AnyValueTagTrait : public std::false_type {};
template<typename T, T V>
struct AnyValueTagTrait<ValueTag<T, V>> : public std::true_type {};
template<typename VTag>
concept AnyValueTag = AnyValueTagTrait<VTag>::value;
template<typename VTag>
concept DerivedValueTag = !AnyValueTag<VTag> && requires {
  typename VTag::Value;
  requires std::same_as<std::decay_t<decltype(VTag::value)>, typename VTag::Value>;
} && std::derived_from<VTag, ValueTag<typename VTag::Value, VTag::value>>;
template<typename VTag, typename T>
concept TypedValueTag = AnyValueTag<VTag> && std::same_as<typename VTag::Value, T>;
template<typename VTag>
concept AnyIndexTag = TypedValueTag<VTag, std::size_t>;
template<typename VTag>
concept AnyBoolTag = TypedValueTag<VTag, bool>;

template<AnyValueTag Tag1, AnyValueTag Tag2>
requires std::same_as<typename Tag1::Value, typename Tag2::Value>
constexpr bool operator==(Tag1 tag1, Tag2 tag2) {
  return tag1.value == tag2.value;
}
template<DerivedValueTag Tag1, DerivedValueTag Tag2>
requires std::same_as<typename Tag1::Value, typename Tag2::Value>
constexpr bool operator==(Tag1 tag1, Tag2 tag2) = delete;
} // namespace thes

#endif // INCLUDE_THESAUROS_TYPES_VALUE_TAG_HPP
