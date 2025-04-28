#include <type_traits>

#include "thesauros/types.hpp"

template<bool tIsValued>
struct TestTag : public thes::BoolTag<tIsValued> {};
using FalseTestTag = TestTag<false>;
inline constexpr FalseTestTag false_test_tag{};
using TrueTestTag = TestTag<true>;
inline constexpr TrueTestTag true_test_tag{};
template<typename T>
struct IsTestTagTrait : public std::false_type {};
template<bool tIsValued>
struct IsTestTagTrait<TestTag<tIsValued>> : public std::true_type {};
template<typename T>
concept AnyTestTag = IsTestTagTrait<T>::value;

template<bool tVal1, bool tVal2>
constexpr bool operator==(TestTag<tVal1> /*tag1*/, TestTag<tVal2> /*tag2*/) {
  return tVal1 == tVal2;
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
