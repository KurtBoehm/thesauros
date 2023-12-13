#ifndef INCLUDE_THESAUROS_RANGES_REDUCE_HPP
#define INCLUDE_THESAUROS_RANGES_REDUCE_HPP

#include <iterator>
#include <numeric>

namespace thes {
template<class TRange, class T, class TBinOp>
inline constexpr T reduce(TRange&& range, T init, TBinOp op) {
  return std::reduce(std::begin(range), std::end(range), init, op);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_RANGES_REDUCE_HPP
