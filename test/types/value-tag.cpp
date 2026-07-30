// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <type_traits>

#include "thesauros/types.hpp"

template<bool IsValued>
struct TestTag : public thes::BoolTag<IsValued> {};
using FalseTestTag = TestTag<false>;
inline constexpr FalseTestTag false_test_tag{};
using TrueTestTag = TestTag<true>;
inline constexpr TrueTestTag true_test_tag{};
template<typename T>
struct IsTestTagTrait : public std::false_type {};
template<bool IsValued>
struct IsTestTagTrait<TestTag<IsValued>> : public std::true_type {};
template<typename T>
concept AnyTestTag = IsTestTagTrait<T>::value;

template<bool V1, bool V2>
constexpr bool operator==(TestTag<V1> /*tag1*/, TestTag<V2> /*tag2*/) {
  return V1 == V2;
}

int main() {
  {
    using Tag = thes::BoolTag<true>;
    [[maybe_unused]] static constexpr Tag tag{};
    static_assert(thes::AnyValueTag<Tag>);
    static_assert(!thes::DerivedValueTag<Tag>);
    static_assert(thes::AnyBoolTag<Tag>);
    static_assert(tag == tag);
  }

  {
    using Tag = TestTag<false>;
    [[maybe_unused]] static constexpr Tag tag{};
    static_assert(!thes::AnyValueTag<Tag>);
    static_assert(thes::DerivedValueTag<Tag>);
    static_assert(tag == tag);
  }
}
