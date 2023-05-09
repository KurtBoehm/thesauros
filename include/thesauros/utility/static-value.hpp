#ifndef INCLUDE_THESAUROS_UTILITY_STATIC_VALUE_HPP
#define INCLUDE_THESAUROS_UTILITY_STATIC_VALUE_HPP

namespace thes {
template<typename T, T tValue>
struct StaticValue {
  using Value = T;
  using Self = StaticValue;
  static constexpr Value value = tValue;

  constexpr operator Value() const noexcept {
    return value;
  }
  constexpr Value operator()() const noexcept {
    return value;
  }
};
template<auto tValue>
using StaticAuto = StaticValue<decltype(tValue), tValue>;

template<typename T, T tValue>
inline constexpr StaticValue<T, tValue> static_value{};
template<auto tValue>
inline constexpr StaticAuto<tValue> static_auto{};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_STATIC_VALUE_HPP
