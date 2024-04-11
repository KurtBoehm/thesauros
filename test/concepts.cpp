#include "thesauros/concepts.hpp"

template<typename>
struct Test;
template<>
struct Test<int> {};

static_assert(thes::IsCompleteTrait<int>::value);
static_assert(!thes::IsCompleteTrait<Test<void>>::value);
static_assert(thes::IsCompleteTrait<Test<int>>::value);
static_assert(!thes::ConstAccess<int>);
static_assert(thes::ConstAccess<const int>);
static_assert(!thes::ConstAccess<int&>);
static_assert(thes::ConstAccess<const int&>);

int main() {}
