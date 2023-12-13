#ifndef INCLUDE_THESAUROS_RANGES_REDUCE_HPP
#define INCLUDE_THESAUROS_RANGES_REDUCE_HPP

#include <utility>

namespace thes {
template<class TRange, class T, class TBinOp>
constexpr T reduce(TRange&& range, T init, TBinOp op) {
  for (decltype(auto) v : std::forward<TRange>(range)) {
    init = op(std::move(init), std::forward<decltype(v)>(v));
  }
  return init;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_RANGES_REDUCE_HPP
