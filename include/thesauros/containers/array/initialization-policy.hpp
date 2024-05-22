#ifndef INCLUDE_THESAUROS_CONTAINERS_ARRAY_INITIALIZATION_POLICY_HPP
#define INCLUDE_THESAUROS_CONTAINERS_ARRAY_INITIALIZATION_POLICY_HPP

#include <memory>

namespace thes {
struct ValueInit {
  template<typename T>
  static constexpr void initialize(T* begin, T* end) {
    std::uninitialized_value_construct(begin, end);
  }
};

struct DefaultInit {
  template<typename T>
  static constexpr void initialize(T* begin, T* end) {
    std::uninitialized_default_construct(begin, end);
  }
};

struct NoInit {
  template<typename T>
  static constexpr void initialize(T* /*begin*/, T* /*end*/) {}
};
} // namespace thes

#endif // INCLUDE_THESAUROS_CONTAINERS_ARRAY_INITIALIZATION_POLICY_HPP
