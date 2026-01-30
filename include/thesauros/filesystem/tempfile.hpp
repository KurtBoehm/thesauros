// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_FILESYSTEM_TEMPFILE_HPP
#define INCLUDE_THESAUROS_FILESYSTEM_TEMPFILE_HPP

#include <array>
#include <cstddef>
#include <filesystem>
#include <random>
#include <stdexcept>
#include <string_view>

#include <pcg_extras.hpp>
#include <pcg_random.hpp>

#include "thesauros/types/primitives.hpp"

namespace thes::fs {
// Create and return a temporary directory
inline std::filesystem::path mkdtemp() {
  static constexpr std::size_t tmp_max = 10'000;
  static constexpr std::size_t prefix_len = 3;
  static constexpr std::size_t random_len = 8;
  static constexpr std::size_t name_len = prefix_len + random_len;
  static constexpr std::string_view tmp_chars = "abcdefghijklmnopqrstuvwxyz0123456789_";

  pcg32 rng{pcg_extras::seed_seq_from<std::random_device>{}};
  std::uniform_int_distribution<u8> dist{0, u8(tmp_chars.size() - 1)};

  const auto tmp_dir = std::filesystem::temp_directory_path();
  std::array<char, name_len> name{'t', 'm', 'p'};

  for (std::size_t i = 0; i < tmp_max; ++i) {
    for (std::size_t j = 0; j < random_len; ++j) {
      name[prefix_len + j] = tmp_chars[dist(rng)];
    }

    const auto dir = tmp_dir / std::string_view{name.data(), name.size()};
    if (std::filesystem::create_directories(dir)) {
      return dir;
    }
  }

  throw std::runtime_error{"Creating a temporary directory failed!"};
}

// A RAII wrapper to create a temporary directory
struct TemporaryDirectory {
  TemporaryDirectory() : path_{mkdtemp()} {}

  TemporaryDirectory(const TemporaryDirectory&) = delete;
  TemporaryDirectory(TemporaryDirectory&&) = delete;
  TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;
  TemporaryDirectory& operator=(TemporaryDirectory&&) = delete;

  ~TemporaryDirectory() {
    std::filesystem::remove_all(path_);
  }

  [[nodiscard]] const std::filesystem::path& path() const noexcept {
    return path_;
  }

private:
  std::filesystem::path path_;
};
} // namespace thes::fs

#endif // INCLUDE_THESAUROS_FILESYSTEM_TEMPFILE_HPP
