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

namespace thes::fs {
/** The generator `mkdtemp` and `TemporaryDirectory` use when none is supplied. */
using DefaultTempRng = std::mt19937;

/**
 * Creates a temporary directory whose name ends in eight characters drawn from `rng`, and returns
 * its path.
 *
 * Uniqueness does not rest on the quality of `rng`: `std::filesystem::create_directories` reports
 * whether it created the directory, so a collision merely costs another attempt, of which up to
 * 10 000 are made before giving up.
 *
 * @throws std::runtime_error if no unused name was found.
 */
template<std::uniform_random_bit_generator Rng>
inline std::filesystem::path mkdtemp(Rng& rng) {
  static constexpr std::size_t tmp_max = 10'000;
  static constexpr std::size_t prefix_len = 3;
  static constexpr std::size_t random_len = 8;
  static constexpr std::size_t name_len = prefix_len + random_len;
  static constexpr std::string_view tmp_chars = "abcdefghijklmnopqrstuvwxyz0123456789_";

  std::uniform_int_distribution<std::size_t> dist{0, tmp_chars.size() - 1};

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

/** `mkdtemp`, using a `DefaultTempRng` seeded from `std::random_device`. */
inline std::filesystem::path mkdtemp() {
  DefaultTempRng rng{std::random_device{}()};
  return mkdtemp(rng);
}

/**
 *An RAII wrapper creating a temporary directory and removing it, with its contents, on destruction.
 */
struct TemporaryDirectory {
  /** Creates a temporary directory, naming it with a `DefaultTempRng`. */
  TemporaryDirectory() : path_{mkdtemp()} {}
  /** Creates a temporary directory, drawing its name from `rng`. */
  template<std::uniform_random_bit_generator Rng>
  explicit TemporaryDirectory(Rng& rng) : path_{mkdtemp(rng)} {}

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
