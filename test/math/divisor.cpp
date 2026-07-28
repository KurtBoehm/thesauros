// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <limits>
#include <utility>

#include "thesauros/thesauros.hpp"

using Size = thes::u64;
static constexpr auto size_max = std::numeric_limits<Size>::max();
using Div = thes::Divisor<Size>;
using Pair = std::pair<Size, Size>;

// 1

static constexpr Div div1{1};

static_assert(0 / div1 == 0);
static_assert(0 % div1 == 0);
static_assert(thes::divmod<Size>(0, div1) == Pair{0, 0});

static_assert(1 / div1 == 1);
static_assert(1 % div1 == 0);
static_assert(thes::divmod<Size>(1, div1) == Pair{1, 0});

static_assert(size_max / div1 == size_max);
static_assert(size_max % div1 == 0);
static_assert(thes::divmod<Size>(size_max, div1) == Pair{size_max, 0});

// 2

static constexpr Div div2{2};

static_assert(0 / div2 == 0);
static_assert(0 % div2 == 0);
static_assert(thes::divmod<Size>(0, div2) == Pair{0, 0});

static_assert(1 / div2 == 0);
static_assert(1 % div2 == 1);
static_assert(thes::divmod<Size>(1, div2) == Pair{0, 1});

static_assert(2 / div2 == 1);
static_assert(2 % div2 == 0);
static_assert(thes::divmod<Size>(2, div2) == Pair{1, 0});

static_assert(size_max / div2 == size_max / 2);
static_assert(size_max % div2 == 1);
static_assert(thes::divmod<Size>(size_max, div2) == Pair{size_max / 2, size_max % 2});

// size_max

static constexpr Div div3{size_max};

static_assert(0 / div3 == 0);
static_assert(0 % div3 == 0);
static_assert(thes::divmod<Size>(0, div3) == Pair{0, 0});

static_assert(1 / div3 == 0);
static_assert(1 % div3 == 1);
static_assert(thes::divmod<Size>(1, div3) == Pair{0, 1});

static_assert((size_max - 1) / div3 == 0);
static_assert((size_max - 1) % div3 == size_max - 1);
static_assert(thes::divmod<Size>(size_max - 1, div3) == Pair{0, size_max - 1});

static_assert(size_max / div3 == 1);
static_assert(size_max % div3 == 0);
static_assert(thes::divmod<Size>(size_max, div3) == Pair{1, 0});

int main() {}
