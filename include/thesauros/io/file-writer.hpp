#ifndef INCLUDE_THESAUROS_IO_FILE_WRITER_HPP
#define INCLUDE_THESAUROS_IO_FILE_WRITER_HPP

#include <cerrno>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <span>
#include <string>
#include <type_traits>
#include <utility>

#include <fmt/core.h>

namespace thes {
struct FileWriterException : public std::exception {
  explicit FileWriterException(std::string msg) : message_(std::move(msg)) {}

  [[nodiscard]] const char* what() const noexcept override {
    return message_.c_str();
  }

private:
  std::string message_;
};

struct FileWriter {
  FileWriter(const FileWriter&) = delete;
  FileWriter(FileWriter&&) = delete;
  FileWriter& operator=(const FileWriter&) = delete;
  FileWriter& operator=(FileWriter&&) = delete;

  explicit FileWriter(std::filesystem::path path)
      : path_(std::move(path)), handle_(std::fopen(path_.c_str(), "wb")) {
    if (handle_ == nullptr) {
      throw FileWriterException(fmt::format("fopen failed: {}", errno));
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
      throw FileWriterException(fmt::format("fwrite failed: {} != {}", written, span.size()));
    }
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
