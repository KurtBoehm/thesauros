#ifndef INCLUDE_THESAUROS_META_STATIC_VALUE_HPP
#define INCLUDE_THESAUROS_META_STATIC_VALUE_HPP

namespace thes {
template<auto tValue>
struct StaticValue {
  using Value = decltype(tValue);
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
inline constexpr StaticValue<tValue> static_value{};
} // namespace thes

#endif // INCLUDE_THESAUROS_META_STATIC_VALUE_HPP
