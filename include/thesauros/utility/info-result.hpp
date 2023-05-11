#ifndef INCLUDE_THESAUROS_UTILITY_INFO_RESULT_HPP
#define INCLUDE_THESAUROS_UTILITY_INFO_RESULT_HPP

#include <concepts>
#include <stdexcept>
#include <utility>

namespace thes {
template<typename TValue, typename TInfo>
struct InfoResult {
  constexpr InfoResult(TValue p_value, TInfo p_info)
      : value{std::move(p_value)}, info{std::move(p_info)} {}

  TValue operator*() const {
    return value;
  }

  constexpr TValue value_if_true() const
  requires std::same_as<TInfo, bool>
  {
    if (!info) {
      throw std::runtime_error("The info is false!");
    }
    return value;
  }
  constexpr TValue value_if_false() const
  requires std::same_as<TInfo, bool>
  {
    if (info) {
      throw std::runtime_error("The info is true!");
    }
    return value;
  }

  TValue value;
  TInfo info;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_INFO_RESULT_HPP
