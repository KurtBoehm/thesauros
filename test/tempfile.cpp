#include <chrono>
#include <cstddef>
#include <filesystem>
#include <thread>

#include "thesauros/filesystem.hpp"
#include "thesauros/format.hpp"

int main() {
  for (std::size_t i = 0; i < 10; ++i) {
    const thes::fs::TemporaryDirectory tmp_dir{};
    fmt::print("folder: {} exists={}\n", tmp_dir.path(), std::filesystem::exists(tmp_dir.path()));
    std::this_thread::sleep_for(std::chrono::seconds{1});
  }
}
