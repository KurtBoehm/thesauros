#ifndef INCLUDE_THESAUROS_IO_DELIMITER_HPP
#define INCLUDE_THESAUROS_IO_DELIMITER_HPP

#include <ostream>
#include <utility>

namespace thes {
template<typename TStr>
struct Delimiter {
  explicit Delimiter(TStr str) : str_(std::forward<TStr>(str)) {}

  friend std::ostream& operator<<(std::ostream& s, Delimiter& delim) {
    if (delim.first_) {
      delim.first_ = false;
    } else {
      s << delim.str_;
    }
    return s;
  }

private:
  TStr str_;
  bool first_ = true;
};
} // namespace thes

#endif // INCLUDE_THESAUROS_IO_DELIMITER_HPP
