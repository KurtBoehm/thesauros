#ifndef INCLUDE_THESAUROS_UTILITY_VALUE_OPTIONAL_HPP
#define INCLUDE_THESAUROS_UTILITY_VALUE_OPTIONAL_HPP

#include <ostream>
#include <utility>

namespace thes {
template<typename T, T tEmpty>
struct ValueOptional {
  using Value = T;
  static constexpr Value empty_value = tEmpty;

  constexpr ValueOptional() = default;
  explicit constexpr ValueOptional(const Value& value) : value_{value} {}
  explicit constexpr ValueOptional(Value&& value) : value_{std::forward<Value>(value)} {}

  void clear() {
    value_ = empty_value;
  }
  void set(const Value& value) {
    assert(value != empty_value);
    value_ = value;
  }
  const Value& value() const {
    assert(value_ != empty_value);
    return value_;
  }
  const Value& operator*() const {
    return value_;
  }

  [[nodiscard]] bool has_value() const {
    return value_ != empty_value;
  }
  [[nodiscard]] bool is_empty() const {
    return value_ == empty_value;
  }

  friend std::ostream& operator<<(std::ostream& stream, const ValueOptional& vo) {
    if (vo.has_value()) {
      stream << vo.get();
    } else {
      stream << "empty";
    }
    return stream;
  }

private:
  Value value_{empty_value};
};
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_VALUE_OPTIONAL_HPP
