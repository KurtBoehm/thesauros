// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_EXECUTION_EXECUTOR_SEQUENTIAL_HPP
#define INCLUDE_THESAUROS_EXECUTION_EXECUTOR_SEQUENTIAL_HPP

#include <cassert>
#include <cstddef>
#include <optional>

#include "thesauros/macropolis/inlining.hpp"

namespace thes {
struct SequentialExecutor {
  explicit constexpr SequentialExecutor() = default;

  [[nodiscard]] static constexpr std::size_t thread_num() {
    return 1;
  }

  THES_ALWAYS_INLINE constexpr void execute(auto task,
                                            std::optional<std::size_t> used_thread_num = {}) const {
    assert(!used_thread_num.has_value() || *used_thread_num == 1);
    task(std::size_t{0});
  }
};
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_EXECUTOR_SEQUENTIAL_HPP
