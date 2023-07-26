#ifndef INCLUDE_THESAUROS_IO_FILE_READER_HPP
#define INCLUDE_THESAUROS_IO_FILE_READER_HPP

#include <array>
#include <cerrno>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <sstream>
#include <string>
#include <utility>

#include "thesauros/containers/dynamic-buffer.hpp"

namespace thes {
struct FileReader {
  struct Exception : public std::exception {
    explicit Exception(std::string msg) : message_(std::move(msg)) {}
    explicit Exception(auto&... args) : message_((std::stringstream{} << ... << args).str()) {}

    [[nodiscard]] const char* what() const noexcept override {
      return message_.c_str();
    }

  private:
    std::string message_;
  };

  explicit FileReader(const std::filesystem::path& path) : handle_(std::fopen(path.c_str(), "rb")) {
    if (handle_ == nullptr) {
      throw Exception("fopen failed: ", errno);
    }
  }
  FileReader(const FileReader&) = delete;
  FileReader(FileReader&&) = delete;
  FileReader& operator=(const FileReader&) = delete;
  FileReader& operator=(FileReader&&) = delete;
  ~FileReader() {
    if (handle_ != nullptr) {
      (void)std::fclose(handle_);
    }
  }

  template<typename T>
  requires std::is_trivial_v<T>
  void read(T* ptr, std::size_t size) {
    const auto ret = std::fread(ptr, 1, size, handle_);
    if (ret != size) {
      throw Exception("fread failed: ", ret, " != ", size);
    }
  }
  void read(DynamicBuffer& buf, std::size_t size) {
    buf.resize(size);
    read(buf.data(), size);
  }
  template<typename T, std::size_t tSize>
  requires std::is_trivial_v<T>
  void read(std::array<T, tSize>& array) {
    read(array.data(), tSize);
  }

  void seek(long offset, int whence) {
    const auto ret = std::fseek(handle_, offset, whence);
    if (ret != 0) {
      throw Exception("fseek failed: ", ret);
    }
  }

  [[nodiscard]] long tell() {
    const auto ret = std::ftell(handle_);
    if (ret == -1) {
      throw Exception("ftell failed: ", errno);
    }
    return ret;
  }

  template<typename T>
  requires std::is_trivial_v<T>
  auto try_pread(T* ptr, std::size_t size, long offset) {
    const auto pre = tell();
    seek(offset, SEEK_SET);
    const auto ret = std::fread(ptr, 1, size, handle_);
    seek(pre, SEEK_SET);
    return ret;
  }
  auto try_pread(DynamicBuffer& buf, std::size_t size, long offset) {
    buf.resize(size);
    const auto ret = try_pread(buf.data(), size, offset);
    buf.resize(ret);
    return ret;
  }

  template<typename T>
  requires std::is_trivial_v<T>
  void pread(T* ptr, std::size_t size, long offset) {
    const auto ret = try_pread(ptr, size, offset);
    if (ret != size) {
      throw Exception("pread failed: ", ret, " != ", size);
    }
  }
  void pread(DynamicBuffer& buf, std::size_t size, long offset) {
    buf.resize(size);
    pread(buf.data(), size, offset);
  }

  [[nodiscard]] std::size_t size() {
    const auto prev = tell();
    seek(0L, SEEK_END);
    const auto size = tell();
    seek(prev, SEEK_SET);
    return static_cast<std::size_t>(size);
  }

private:
  FILE* handle_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_IO_FILE_READER_HPP
