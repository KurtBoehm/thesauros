// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <concepts>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <random>
#include <set>

#include "thesauros/filesystem/tempfile.hpp"
#include "thesauros/test/test.hpp"

namespace {
THES_TEST_CASE("TemporaryDirectory creates a directory and removes it again", "[filesystem]") {
  std::filesystem::path path;
  {
    const thes::fs::TemporaryDirectory dir{};
    path = dir.path();
    THES_REQUIRE(std::filesystem::exists(path));
    THES_CHECK(std::filesystem::is_directory(path));
    THES_CHECK(path.filename().string().starts_with("tmp"));
  }
  THES_CHECK(!std::filesystem::exists(path));
}

THES_TEST_CASE("TemporaryDirectory removes its contents too", "[filesystem]") {
  std::filesystem::path path;
  {
    const thes::fs::TemporaryDirectory dir{};
    path = dir.path();
    std::ofstream{dir.path() / "file.txt"} << "content";
    THES_REQUIRE(std::filesystem::exists(dir.path() / "file.txt"));
  }
  THES_CHECK(!std::filesystem::exists(path));
}

THES_TEST_CASE("mkdtemp accepts any uniform random bit generator", "[filesystem]") {
  // The point of the template parameter: a caller can supply a generator of their own instead of
  // the `DefaultTempRng`. These two engines differ from that default and from each other, so they
  // exercise the parameter rather than just the default path.
  std::minstd_rand lcg{std::random_device{}()};
  const thes::fs::TemporaryDirectory lcg_dir{lcg};
  THES_CHECK(std::filesystem::is_directory(lcg_dir.path()));

  std::ranlux24 ranlux{std::random_device{}()};
  const auto ranlux_path = thes::fs::mkdtemp(ranlux);
  THES_CHECK(std::filesystem::is_directory(ranlux_path));
  std::filesystem::remove_all(ranlux_path);

  static_assert(!std::same_as<std::minstd_rand, thes::fs::DefaultTempRng>);
  static_assert(!std::same_as<std::ranlux24, thes::fs::DefaultTempRng>);
}

THES_TEST_CASE("Simultaneously existing temporary directories are distinct", "[filesystem]") {
  static constexpr std::size_t count = 16;

  std::set<std::filesystem::path> paths;
  for (std::size_t i = 0; i < count; ++i) {
    const auto path = thes::fs::mkdtemp();
    THES_REQUIRE(std::filesystem::is_directory(path));
    paths.insert(path);
  }
  THES_CHECK(paths.size() == count);

  for (const auto& path : paths) {
    std::filesystem::remove_all(path);
  }
}
} // namespace

THES_TEST_MAIN()
