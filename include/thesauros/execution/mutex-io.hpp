#ifndef INCLUDE_THESAUROS_EXECUTION_MUTEX_IO_HPP
#define INCLUDE_THESAUROS_EXECUTION_MUTEX_IO_HPP

#include <ios>
#include <iostream>
#include <mutex>
#include <ostream>
#include <utility>

namespace thes {
template<typename TStream>
struct MutexStream {
  MutexStream(TStream&& stream, std::mutex& mutex)
      : stream_{std::forward<TStream>(stream)}, mutex_{mutex} {
    mutex_.lock();
  }
  MutexStream(const MutexStream&) = delete;
  MutexStream(MutexStream&&) = delete;
  MutexStream& operator=(const MutexStream&) = delete;
  MutexStream& operator=(MutexStream&&) = delete;

  ~MutexStream() {
    mutex_.unlock();
  }

  template<typename T>
  MutexStream& operator<<(const T& t) {
    stream_ << t;
    return *this;
  }
  MutexStream& operator<<(std::ios_base& (*func)(std::ios_base&)) {
    stream_ << func;
    return *this;
  }
  MutexStream& operator<<(std::ios& (*func)(std::ios&)) {
    stream_ << func;
    return *this;
  }
  MutexStream& operator<<(std::ostream& (*func)(std::ostream&)) {
    stream_ << func;
    return *this;
  }

private:
  TStream stream_;
  std::mutex& mutex_;
};

inline std::mutex& parallel_io_mutex() {
  static std::mutex mutex{};
  return mutex;
}

inline MutexStream<std::ostream&> mutex_cout() {
  return MutexStream<std::ostream&>{std::cout, parallel_io_mutex()};
}
} // namespace thes

#endif // INCLUDE_THESAUROS_EXECUTION_MUTEX_IO_HPP
