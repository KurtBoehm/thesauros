#ifndef INCLUDE_THESAUROS_IO_FILE_WRITER_HPP
#define INCLUDE_THESAUROS_IO_FILE_WRITER_HPP

#include <cerrno>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <sstream>
#include <string>
#include <utility>

namespace thes {
struct FileWriter {
  struct Exception : public std::exception {
    explicit Exception(std::string msg) : message_(std::move(msg)) {}
    explicit Exception(auto&... args) : message_((std::stringstream{} << ... << args).str()) {}

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

  explicit FileWriter(const std::filesystem::path& path) : handle_(std::fopen(path.c_str(), "wb")) {
    if (handle_ == nullptr) {
      throw Exception("fopen failed: ", errno);
    }
  }
  ~FileWriter() {
    if (handle_ != nullptr) {
      (void)std::fclose(handle_);
    }
  }

  template<typename T>
  requires std::is_trivial_v<T>
  void write(const T* data, std::size_t num) {
    const auto written = std::fwrite(data, sizeof(T), num, handle_);
    if (written != num) {
      throw Exception("fwrite failed: ", written, " != ", num);
    }
  }

private:
  FILE* handle_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_IO_FILE_WRITER_HPP
