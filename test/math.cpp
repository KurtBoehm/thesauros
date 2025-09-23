// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <cmath>
#include <limits>

#include "thesauros/math.hpp"
#include "thesauros/static-ranges.hpp"
#include "thesauros/test.hpp"
#include "thesauros/types.hpp"

static_assert(thes::add_max<unsigned char>(1, 4, 3) == 3);
static_assert(thes::add_max<unsigned char>(1, 2, 4) == 3);
static_assert(thes::add_max<unsigned char>(128, 128, 254) == 254);

static_assert(thes::sub_min<unsigned>(5, 3, 2) == 2);
static_assert(thes::sub_min<unsigned>(5, 7, 2) == 2);

template<typename T>
void run() {
  auto test = [](T v, bool is_finite) {
    if (is_finite) {
      THES_ASSERT(thes::is_finite(v));
      THES_ASSERT(thes::make_finite(v) == v);
    } else {
      THES_ASSERT(!thes::is_finite(v));
      THES_ASSERT(thes::make_finite(v) == T{});
    }
  };

  test(T{1}, true);
  test(+T{0}, true);
  test(-T{0}, true);
  test(std::numeric_limits<T>::denorm_min(), true);
  test(std::numeric_limits<T>::epsilon(), true);
  test(std::numeric_limits<T>::lowest(), true);
  test(std::numeric_limits<T>::max(), true);
  test(std::numeric_limits<T>::min(), true);
  test(std::numeric_limits<T>::round_error(), true);
  test(std::numeric_limits<T>::infinity(), false);
  test(-std::numeric_limits<T>::infinity(), false);
  test(std::numeric_limits<T>::quiet_NaN(), false);
  test(std::numeric_limits<T>::signaling_NaN(), false);
  test(T(std::nan("BEEF")), false);

  static_assert(thes::isqrt_floor(0U) == 0);
  static_assert(thes::isqrt_ceil(0U) == 0);
  thes::star::iota<1, 200> | thes::star::for_each([](auto i) {
    constexpr auto root_lb = thes::isqrt_floor(i.value);
    static_assert(root_lb * root_lb <= i && (root_lb + 1) * (root_lb + 1) > i);
    constexpr thes::u64 root_ub = thes::isqrt_ceil(i.value);
    static_assert((root_ub - 1) * (root_ub - 1) < i && root_ub * root_ub >= i);
  });

  for (thes::u64 i = 0; i < (thes::u64{1} << 25U); ++i) {
    const thes::u64 root_lb = thes::isqrt_floor(i);
    THES_ALWAYS_ASSERT(root_lb * root_lb <= i && (root_lb + 1) * (root_lb + 1) > i);
  }

  for (thes::u64 i = 1; i < (thes::u64{1} << 25U); ++i) {
    const thes::u64 root_ub = thes::isqrt_ceil(i);
    THES_ALWAYS_ASSERT((root_ub - 1) * (root_ub - 1) < i && root_ub * root_ub >= i);
  }
}

int main() {
  run<thes::f32>();
  run<thes::f64>();
}
