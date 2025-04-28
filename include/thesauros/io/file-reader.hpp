#ifndef INCLUDE_THESAUROS_IO_FILE_READER_HPP
#define INCLUDE_THESAUROS_IO_FILE_READER_HPP

#include <array>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <span>
#include <type_traits>

#include <fmt/core.h>

#include "thesauros/containers/dynamic-buffer.hpp"
#include "thesauros/io/file.hpp"
#include "thesauros/macropolis/inlining.hpp"
#include "thesauros/utility/integer-cast.hpp"
#include "thesauros/utility/type-tag.hpp"

namespace thes {
struct FileReader {
  explicit FileReader(const std::filesystem::path& path) : handle_(std::fopen(path.c_str(), "rb")) {
    if (handle_ == nullptr) {
      throw FileException{fmt::format("fopen failed: {}", errno)};
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
  THES_ALWAYS_INLINE std::size_t try_read(std::span<T> span) {
    const auto ret = std::fread(span.data(), sizeof(T), span.size(), handle_);
    const auto err = std::ferror(handle_);
    if (err) {
      throw FileException{fmt::format("fread failed ({}), ret={}, size={}", err, ret, span.size())};
    }
    return ret;
  }

  template<typename T>
  requires std::is_trivial_v<T>
  THES_ALWAYS_INLINE void read(std::span<T> span) {
    const auto ret = std::fread(span.data(), sizeof(T), span.size(), handle_);
    if (ret != span.size()) {
      throw FileException{fmt::format("fread failed: {} != {}", ret, span.size())};
    }
  }
  THES_ALWAYS_INLINE void read(DynamicBuffer& buf, std::size_t size) {
    buf.resize(size);
    read(buf.span());
  }
  template<typename T, std::size_t tSize>
  requires std::is_trivial_v<T>
  THES_ALWAYS_INLINE void read(std::array<T, tSize>& array) {
    read(std::span{array.data(), array.size()});
  }
  template<typename T>
  requires std::is_trivial_v<T>
  THES_ALWAYS_INLINE T read(TypeTag<T> /*tag*/) {
    T value{};
    read(std::span{&value, 1});
    return value;
  }

  void read_full(BufferLike auto& buf) {
    if (const auto off = tell(); off != 0) {
      throw FileException{fmt::format("read_full has to start at the beginning, not at {}!", off)};
    }
    buf.resize(size());
    buf.resize(try_read(std::span{buf.data(), buf.size()}));
  }
  template<BufferLike TBuf>
  TBuf read_full(TypeTag<TBuf> /*tag*/ = {}) {
    TBuf buf{};
    read_full(buf);
    return buf;
  }

  THES_ALWAYS_INLINE void seek(long offset, Seek whence) {
    const auto ret = std::fseek(handle_, offset, int(whence));
    if (ret != 0) {
      throw FileException{fmt::format("fseek failed: {}", ret)};
    }
  }

  [[nodiscard]] THES_ALWAYS_INLINE long tell() {
    const auto ret = std::ftell(handle_);
    if (ret == -1) {
      throw FileException{fmt::format("ftell failed: {}", errno)};
    }
    return ret;
  }

  template<typename T>
  requires std::is_trivial_v<T>
  std::size_t try_pread(std::span<T> span, long offset) {
    const auto pre = tell();
    seek(offset, Seek::set);
    const auto ret = try_read(span);
    seek(pre, Seek::set);
    return ret;
  }
  std::size_t try_pread(DynamicBuffer& buf, std::size_t size, long offset) {
    buf.resize(size);
    const auto ret = try_pread(buf.span(), offset);
    buf.resize(ret);
    return ret;
  }

  template<typename T>
  requires std::is_trivial_v<T>
  void pread(std::span<T> span, long offset) {
    const auto ret = try_pread(span, offset);
    if (ret != span.size()) {
      throw FileException{fmt::format("pread failed: {} != {}", ret, span.size())};
    }
  }
  template<typename T>
  requires std::is_trivial_v<T>
  T pread(long offset, TypeTag<T> /*tag*/) {
    T value{};
    pread(std::span{&value, 1}, offset);
    return value;
  }
  void pread(DynamicBuffer& buf, std::size_t size, long offset) {
    buf.resize(size);
    pread(buf.span(), offset);
  }

  [[nodiscard]] std::size_t size() {
    const auto prev = tell();
    seek(0L, Seek::end);
    const auto size = tell();
    seek(prev, Seek::set);
    return *safe_cast<std::size_t>(size);
  }

  [[nodiscard]] bool end_of_file() const {
    return std::feof(handle_) != 0;
  }

  [[nodiscard]] FILE* handle() const {
    return handle_;
  }

private:
  FILE* handle_;
};

template<BufferLike TBuf>
void read_file(TBuf& dst, const std::filesystem::path& path) {
  FileReader{path}.read_full(dst);
}
template<BufferLike TBuf>
TBuf read_file(const std::filesystem::path& path, TypeTag<TBuf> /*tag*/ = {}) {
  return FileReader{path}.read_full<TBuf>();
}
} // namespace thes

#endif // INCLUDE_THESAUROS_IO_FILE_READER_HPP
