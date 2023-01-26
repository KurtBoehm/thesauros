#ifndef INCLUDE_THESAUROS_META_STATIC_VALUE_HPP
#define INCLUDE_THESAUROS_META_STATIC_VALUE_HPP

namespace thes {
template<auto tValue>
struct StaticValue {
  using value_type = decltype(tValue);
  using type = StaticValue;
  static constexpr value_type value = tValue;

  constexpr operator value_type() const noexcept {
    return value;
  }
  constexpr value_type operator()() const noexcept {
    return value;
  }
};

template<auto tValue>
inline constexpr StaticValue<tValue> static_value{};
} // namespace thes

#endif // INCLUDE_THESAUROS_META_STATIC_VALUE_HPP
