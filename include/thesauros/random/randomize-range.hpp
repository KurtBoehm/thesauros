#ifndef INCLUDE_THESAUROS_RANDOM_RANDOMIZE_RANGE_HPP
#define INCLUDE_THESAUROS_RANDOM_RANDOMIZE_RANGE_HPP

#include <bit>
#include <limits>

namespace thes {
template<typename T>
struct RangeRandomizer {
  explicit RangeRandomizer(T size) : size_(size) {}

  constexpr T transform(T x) const {
    constexpr auto mul = static_cast<T>(12605985483714917081ULL);

    auto part = [this](const T y) {
      const T splits = size_ & ~y;
      // y < size_ => split != 0
      const T m = (std::numeric_limits<T>::max() >> std::countl_zero(splits)) >> 1;

      const T z = (y ^ (y >> T{1})) * mul;
      return (y & ~m) | (z & m);
    };

    x = part(x);
    x = size_ - x - 1;
    x = part(x);

    return x;
  }

private:
  T size_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_RANDOM_RANDOMIZE_RANGE_HPP
