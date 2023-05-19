#ifndef INCLUDE_THESAUROS_EXECUTION_SYSTEM_AFFINITY_HPP
#define INCLUDE_THESAUROS_EXECUTION_SYSTEM_AFFINITY_HPP

#include <cstddef>
#include <thread>

#include <pthread.h>

#include "tl/expected.hpp"

#include "thesauros/utility/as-expected.hpp"

namespace thes {
struct CPUSet {
  CPUSet() {
    CPU_ZERO(&cpu_set_);
  }

  CPUSet(const CPUSet& other) = delete;
  CPUSet(CPUSet&& other) = default;
  CPUSet& operator=(const CPUSet& other) = delete;
  CPUSet& operator=(CPUSet&& other) = default;

  ~CPUSet() = default;

  static CPUSet single_set(std::size_t i) {
    CPUSet output{};
    output.set(i);
    return output;
  }

  void set(const std::size_t i) {
    CPU_SET(i, &cpu_set_);
  }

  [[nodiscard]] const cpu_set_t& base() const {
    return cpu_set_;
  }

private:
  cpu_set_t cpu_set_{};
};

inline tl::expected<void, int> set_affinity(std::thread& thread, const CPUSet& cpu_set) {
  const auto ret =
    pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &cpu_set.base());
  return as_expected(ret);
}
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_SYSTEM_AFFINITY_HPP
