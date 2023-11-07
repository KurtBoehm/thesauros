#ifndef INCLUDE_THESAUROS_EXECUTION_CONCURRENCY_HPP
#define INCLUDE_THESAUROS_EXECUTION_CONCURRENCY_HPP

#include <charconv>
#include <cstddef>
#include <fstream>
#include <string>
#include <string_view>

namespace thes {
inline std::size_t physical_concurrency() noexcept {
  constexpr std::string_view cpu_cores = "cpu cores";

  std::ifstream proc_cpuinfo("/proc/cpuinfo");

  std::string line{};
  while (std::getline(proc_cpuinfo, line, '\n')) {
    if (line.empty()) {
      continue;
    }

    std::size_t key_size = line.find(':');
    if (key_size == std::string::npos) {
      continue;
    }

    std::string_view line_view{line};
    std::string_view key = line_view.substr(0, key_size);
    std::size_t end = key.size();
    for (; end > 0 && key[end - 1] == '\t'; --end) {
    }
    key.remove_suffix(key.size() - end);

    if (key == cpu_cores) {
      auto value_view = line_view.substr(key_size + 2);

      std::size_t output{};
      const auto res = std::from_chars(value_view.begin(), value_view.end(), output);
      if (res.ptr != value_view.end()) {
        continue;
      }

      return output;
    }
  }

  // fallback
  // return std::thread::hardware_concurrency();
  return 0;
}
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_CONCURRENCY_HPP
