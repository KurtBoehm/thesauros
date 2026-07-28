// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

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
