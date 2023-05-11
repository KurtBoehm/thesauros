#ifndef INCLUDE_THESAUROS_UTILITY_INFO_RESULT_HPP
#define INCLUDE_THESAUROS_UTILITY_INFO_RESULT_HPP

#include <utility>

namespace thes {
template<typename TValue, typename TInfo>
struct InfoResult {
  constexpr InfoResult(TValue value, TInfo info) : value{std::move(value)}, info{std::move(info)} {}

  TValue operator*() const {
    return value;
  }

  TValue value;
  TInfo info;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_INFO_RESULT_HPP
