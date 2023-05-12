#ifndef INCLUDE_THESAUROS_UTILITY_INFO_RESULT_HPP
#define INCLUDE_THESAUROS_UTILITY_INFO_RESULT_HPP

#include <concepts>
#include <stdexcept>
#include <utility>

namespace thes {
template<typename TValue, typename TInfo, TInfo tValid>
struct InfoResult {
  constexpr InfoResult(TValue value, TInfo info)
      : value_{std::move(value)}, info_{std::move(info)} {}

  [[nodiscard]] constexpr TValue operator*() const {
    return value_;
  }

  [[nodiscard]] constexpr bool is_valid() const {
    return info_ == tValid;
  }
  constexpr TValue valid_value() const {
    if (!is_valid()) {
      throw std::runtime_error("The value is invalid!");
    }
    return value_;
  }

private:
  TValue value_;
  TInfo info_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_INFO_RESULT_HPP
