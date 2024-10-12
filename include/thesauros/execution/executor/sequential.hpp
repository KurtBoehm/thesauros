#ifndef INCLUDE_THESAUROS_EXECUTION_EXECUTOR_SEQUENTIAL_HPP
#define INCLUDE_THESAUROS_EXECUTION_EXECUTOR_SEQUENTIAL_HPP

#include <cstddef>

#include "thesauros/utility/inlining.hpp"

namespace thes {
struct SequentialExecutor {
  explicit constexpr SequentialExecutor() = default;

  [[nodiscard]] constexpr static std::size_t thread_num() {
    return 1;
  }

  THES_ALWAYS_INLINE constexpr void execute(auto task) const {
    task(std::size_t{0});
  }
};
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_EXECUTOR_SEQUENTIAL_HPP
