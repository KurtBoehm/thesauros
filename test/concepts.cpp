#include "thesauros/concepts.hpp"

template<typename>
struct Test;
template<>
struct Test<int> {};

static_assert(thes::IsCompleteTrait<int>::value);
static_assert(!thes::IsCompleteTrait<Test<void>>::value);
static_assert(thes::IsCompleteTrait<Test<int>>::value);

int main() {}
