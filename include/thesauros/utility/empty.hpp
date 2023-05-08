#ifndef INCLUDE_THESAUROS_UTILITY_EMPTY_HPP
#define INCLUDE_THESAUROS_UTILITY_EMPTY_HPP

#include <tuple>
#include <utility>

namespace thes {
struct Empty {
  [[nodiscard]] constexpr static bool has_value() {
    return false;
  }

  [[nodiscard]] bool operator==(const Empty& other) const = default;
};

template<typename TFun, typename TArgTuple>
inline constexpr auto apply_empty(TFun&& fun, TArgTuple&& args) {
  auto impl = [&] { return std::apply(std::forward<TFun>(fun), std::forward<TArgTuple>(args)); };
  using Type = decltype(impl());

  if constexpr (std::is_void_v<Type>) {
    impl();
    return Empty{};
  } else {
    return impl();
  }
}
} // namespace thes

#endif // INCLUDE_THESAUROS_UTILITY_EMPTY_HPP
