#ifndef INCLUDE_THESAUROS_FUNCTIONAL_BINARY_HPP
#define INCLUDE_THESAUROS_FUNCTIONAL_BINARY_HPP

#include <algorithm>
#include <utility>

namespace thes {
struct Maximum {
  template<typename T1, typename T2>
  constexpr decltype(auto) operator()(T1&& v1, T2&& v2) const {
    return std::max(std::forward<T1>(v1), std::forward<T2>(v2));
  }
};
struct Minimum {
  template<typename T1, typename T2>
  constexpr decltype(auto) operator()(T1&& v1, T2&& v2) const {
    return std::min(std::forward<T1>(v1), std::forward<T2>(v2));
  }
};
} // namespace thes

#endif // INCLUDE_THESAUROS_FUNCTIONAL_BINARY_HPP
