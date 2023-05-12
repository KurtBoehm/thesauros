#ifndef INCLUDE_THESAUROS_UTILITY_NUMERIC_INFO_HPP
#define INCLUDE_THESAUROS_UTILITY_NUMERIC_INFO_HPP

#include <climits>
#include <limits>

namespace thes {
template<typename T>
struct NumericInfo {
  static constexpr auto byte_num = sizeof(T);
  static constexpr auto bit_num = CHAR_BIT * byte_num;

  static constexpr unsigned digits = static_cast<unsigned>(std::numeric_limits<T>::digits);
  static constexpr T max = std::numeric_limits<T>::max();
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_NUMERIC_INFO_HPP
