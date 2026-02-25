// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

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
