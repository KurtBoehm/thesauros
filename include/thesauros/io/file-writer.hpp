// This file is part of https://github.com/KurtBoehm/thesauros.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef INCLUDE_THESAUROS_IO_FILE_WRITER_HPP
#define INCLUDE_THESAUROS_IO_FILE_WRITER_HPP

#include <cerrno>
#include <cstdio>
#include <filesystem>
#include <span>
#include <type_traits>
#include <utility>

#include "thesauros/format/fmtlib.hpp"
#include "thesauros/io/file.hpp"

namespace thes {
struct FileWriter {
  FileWriter(const FileWriter&) = delete;
  FileWriter(FileWriter&&) = delete;
  FileWriter& operator=(const FileWriter&) = delete;
  FileWriter& operator=(FileWriter&&) = delete;

  explicit FileWriter(std::filesystem::path path)
      : path_(std::move(path)), handle_(std::fopen(path_string(path_).c_str(), "wb+")) {
    if (handle_ == nullptr) {
      throw FileException(fmt::format("fopen failed: {}", errno));
    }
  }
  ~FileWriter() {
    if (handle_ != nullptr) {
      (void)std::fclose(handle_);
    }
  }

  [[nodiscard]] const std::filesystem::path& path() const {
    return path_;
  }

  template<typename T>
  requires std::is_trivial_v<std::decay_t<T>>
  void write(std::span<T> span) {
    const auto written = std::fwrite(span.data(), sizeof(T), span.size(), handle_);
    if (written != span.size()) {
      throw FileException(fmt::format("fwrite failed: {} != {}", written, span.size()));
    }
  }
  template<typename T>
  void write(const T& value) {
    write(std::span{&value, 1});
  }

  void seek(long offset, Seek whence) {
    const auto ret = std::fseek(handle_, offset, int(whence));
    if (ret != 0) {
      throw FileException(fmt::format("fseek failed: {}", ret));
    }
  }

  [[nodiscard]] long tell() {
    const auto ret = std::ftell(handle_);
    if (ret == -1) {
      throw FileException(fmt::format("ftell failed: {}", errno));
    }
    return ret;
  }

  [[nodiscard]] FILE* handle() const {
    return handle_;
  }

private:
  std::filesystem::path path_;
  FILE* handle_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_IO_FILE_WRITER_HPP
