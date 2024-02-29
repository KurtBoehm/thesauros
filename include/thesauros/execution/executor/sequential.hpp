#ifndef INCLUDE_THESAUROS_EXECUTION_EXECUTOR_SEQUENTIAL_HPP
#define INCLUDE_THESAUROS_EXECUTION_EXECUTOR_SEQUENTIAL_HPP

#include <cstddef>

namespace thes {
struct SequentialExecutor {
  explicit constexpr SequentialExecutor() = default;

  [[nodiscard]] constexpr static std::size_t size() {
    return 1;
  }

  constexpr void execute(auto task) const {
    task(std::size_t{0});
  }
};
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_EXECUTOR_SEQUENTIAL_HPP
