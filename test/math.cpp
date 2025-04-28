#include <cmath>
#include <limits>

#include "thesauros/math.hpp"
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
}

int main() {
  run<thes::f32>();
  run<thes::f64>();
}
