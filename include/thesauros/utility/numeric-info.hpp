#ifndef INCLUDE_THESAUROS_UTILITY_NUMERIC_INFO_HPP
#define INCLUDE_THESAUROS_UTILITY_NUMERIC_INFO_HPP

#include <climits>

namespace thes {
template<typename T>
struct NumericInfo {
  static constexpr auto byte_num = sizeof(T);
  static constexpr auto bit_num = CHAR_BIT * byte_num;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_NUMERIC_INFO_HPP
