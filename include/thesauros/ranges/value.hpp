#ifndef INCLUDE_THESAUROS_RANGES_VALUE_HPP
#define INCLUDE_THESAUROS_RANGES_VALUE_HPP

#include <utility>

namespace thes {
template<typename TIt>
struct ValueRange {
  ValueRange(TIt begin, TIt end) : begin_(std::move(begin)), end_(std::move(end)) {}

  TIt begin() const {
    return begin_;
  }
  TIt end() const {
    return end_;
  }

private:
  TIt begin_;
  TIt end_;
};

template<typename TIt>
inline constexpr ValueRange<TIt> value_range(TIt begin, TIt end) {
  return {std::move(begin), std::move(end)};
}
} // namespace thes

#endif // INCLUDE_THESAUROS_RANGES_VALUE_HPP
