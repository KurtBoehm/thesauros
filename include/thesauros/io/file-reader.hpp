#ifndef INCLUDE_THESAUROS_IO_FILE_READER_HPP
#define INCLUDE_THESAUROS_IO_FILE_READER_HPP

#include <array>
#include <cerrno>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <span>
#include <sstream>
#include <string>
#include <utility>

#include "thesauros/containers/dynamic-buffer.hpp"
#include "thesauros/utility/type-tag.hpp"

namespace thes {
struct FileReader {
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
  void read(std::span<T> span) {
    const auto ret = std::fread(span.data(), sizeof(T), span.size(), handle_);
    if (ret != span.size()) {
      throw Exception("fread failed: ", ret, " != ", span.size());
    }
  }
  void read(DynamicBuffer& buf, std::size_t size) {
    buf.resize(size);
    read(std::span{buf.data(), buf.size()});
  }
  template<typename T, std::size_t tSize>
  requires std::is_trivial_v<T>
  void read(std::array<T, tSize>& array) {
    read(std::span{array.data(), array.size()});
  }
  template<typename T>
  requires std::is_trivial_v<T>
  T read(TypeTag<T> /*tag*/) {
    T value{};
    read(std::span{&value, 1});
    return value;
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
  auto try_pread(std::span<T> span, long offset) {
    const auto pre = tell();
    seek(offset, SEEK_SET);
    const auto ret = std::fread(span.data(), sizeof(T), span.size(), handle_);
    seek(pre, SEEK_SET);
    return ret;
  }
  auto try_pread(DynamicBuffer& buf, std::size_t size, long offset) {
    buf.resize(size);
    const auto ret = try_pread(std::span{buf.data(), buf.size()}, offset);
    buf.resize(ret);
    return ret;
  }

  template<typename T>
  requires std::is_trivial_v<T>
  void pread(std::span<T> span, long offset) {
    const auto ret = try_pread(span, offset);
    if (ret != span.size()) {
      throw Exception("pread failed: ", ret, " != ", span.size());
    }
  }
  void pread(DynamicBuffer& buf, std::size_t size, long offset) {
    buf.resize(size);
    pread(std::span{buf.data(), buf.size()}, offset);
  }

  [[nodiscard]] std::size_t size() {
    const auto prev = tell();
    seek(0L, SEEK_END);
    const auto size = tell();
    seek(prev, SEEK_SET);
    return static_cast<std::size_t>(size);
  }

  [[nodiscard]] FILE* handle() const {
    return handle_;
  }

private:
  FILE* handle_;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_IO_FILE_READER_HPP
