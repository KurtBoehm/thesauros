#ifndef INCLUDE_THESAUROS_IO_FILE_WRITER_HPP
#define INCLUDE_THESAUROS_IO_FILE_WRITER_HPP

#include <cerrno>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <span>
#include <sstream>
#include <string>
#include <utility>

namespace thes {
struct FileWriter {
  struct Exception : public std::exception {
    explicit Exception(std::string msg) : message_(std::move(msg)) {}
    explicit Exception(const auto&... args)
        : message_((std::stringstream{} << ... << args).str()) {}

    [[nodiscard]] const char* what() const noexcept override {
      return message_.c_str();
    }

  private:
    std::string message_;
  };

  FileWriter(const FileWriter&) = delete;
  FileWriter(FileWriter&&) = delete;
  FileWriter& operator=(const FileWriter&) = delete;
  FileWriter& operator=(FileWriter&&) = delete;

  explicit FileWriter(std::filesystem::path path)
      : path_(std::move(path)), handle_(std::fopen(path_.c_str(), "wb")) {
    if (handle_ == nullptr) {
      throw Exception("fopen failed: ", errno);
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
  requires std::is_trivial_v<T>
  void write(std::span<const T> span) {
    const auto written = std::fwrite(span.data(), sizeof(T), span.size(), handle_);
    if (written != span.size()) {
      throw Exception("fwrite failed: ", written, " != ", span.size());
    }
  }

private:
  std::filesystem::path path_;
  FILE* handle_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_IO_FILE_WRITER_HPP
